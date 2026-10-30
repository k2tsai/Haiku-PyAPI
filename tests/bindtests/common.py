#!/usr/bin/env python3
#
# common.py — shared harness for the Haiku-PyAPI binding tests (run where the
# bindings are built).
#
# Tests run against the SYMLINK FARM (/boot/home/prefixrun/Be -> the shared
# build dir), NOT /boot/home/Be — so a rebuilt .so is picked up without a
# re-copy, and tests never shadow each other. Import order matters (base
# modules first): use closure() for anything with cross-module bases.
#
# No pytest in the build environment (no pip): each test file is a plain
# script printing JSON result lines + an exit code; run_all.py aggregates.
#
import importlib
import json
import os
import subprocess
import sys
import types

# Configuration (env-overridable; defaults match the standard VM layout —
# the suite dir is otherwise self-contained and relocatable):
#   BE_PKG_DIR           the Be package dir to import (the .so farm)
#   BINDTESTS_SCRATCH    scratch dir for test files
#   BINDTESTS_ENV        alternate .env location (default: ./.env)
#   PYAPI_BINDINGS       bindings source tree (selectmods analysis)
#   PYAPI_STOCK_INIT     stock import-order file
#   PYAPI_BUDGET         budget.py for selectmods --budget
#   PYAPI_SELECTMODS_DIR fallback dir for selectmods.py
#
# .env support: stdlib-only loader (no pip/python-dotenv in the build env).
# Precedence is the dotenv standard: values already in the environment
# ALWAYS win over .env entries.
HERE = os.path.dirname(os.path.abspath(__file__))


def _load_dotenv():
    for path in (os.environ.get("BINDTESTS_ENV"),
                 os.path.join(HERE, ".env")):
        if path and os.path.isfile(path):
            for line in open(path, errors="replace"):
                line = line.strip()
                if not line or line.startswith("#") or "=" not in line:
                    continue
                k, _, v = line.partition("=")
                k = k.strip()
                v = v.strip().strip('"').strip("'")
                os.environ.setdefault(k, v)   # env wins over .env
            break


_load_dotenv()

def _default_farm():
    p = "/boot/home/prefixrun/Be"
    return p if os.path.isdir(p) else "/boot/home/Be"

FARM = os.environ.get("BE_PKG_DIR", _default_farm())
SCRATCH = os.environ.get("BINDTESTS_SCRATCH", os.path.join(HERE, "scratch"))

os.makedirs(SCRATCH, exist_ok=True)

RESULTS = []
ABSENT = set()          # module names whose .so isn't in FARM (not built)


class Absent(Exception):
    """a row used a module that didn't build (an _Absent sentinel) → ABSENT."""
    def __init__(self, name):
        self.name = name
        super().__init__(name)


class _Absent:
    """sentinel bound for a name whose module didn't build (see bind_names);
    using it — calling, attribute access, comparing, hashing — raises Absent so
    the row is reported ABSENT, not a crash or a silent mis-comparison. The
    dunders are defined explicitly (special methods are looked up on the type,
    not via __getattr__)."""
    def __init__(self, name): self.name = name
    def __call__(self, *a, **k): raise Absent(self.name)
    def __getattr__(self, k): raise Absent(self.name)
    def __eq__(self, other): raise Absent(self.name)
    def __ne__(self, other): raise Absent(self.name)
    def __hash__(self): raise Absent(self.name)


def load(mods):
    """Seed an empty Be package over the farm; import each module that has a
    built `.so` — a module that didn't build is recorded in ABSENT (not
    imported, not fatal). -> the Be package (present modules' public names)."""
    global ABSENT
    ABSENT = set()
    pkg = types.ModuleType("Be")
    pkg.__path__ = [FARM]
    sys.modules["Be"] = pkg
    for m in mods:
        if not os.path.isfile(os.path.join(FARM, m + ".so")):
            ABSENT.add(m)
            continue
        mod = importlib.import_module("Be." + m)
        for k, v in vars(mod).items():
            if not k.startswith("_"):
                setattr(pkg, k, v)
    return pkg


def bind_names(Be, names):
    """Bind a whitespace-separated list of `names` for the caller: present
    ones from Be, absent ones as _Absent sentinels (so a module that didn't
    build doesn't crash the file's import). Use:
        globals().update(common.bind_names(Be, "BNode BFile B_OK ..."))"""
    return {n: (getattr(Be, n) if hasattr(Be, n) else _Absent(n)) for n in names.split()}


def absent(name, why):
    RESULTS.append({"test": name, "status": "ABSENT", "detail": why})


def closure(*wanted):
    """selectmods.analyze() dependency closure, stock-ordered.
    `wanted` may be module names OR C++ class names (resolved via the
    analyzer's type map)."""
    # selectmods sits with the suite; fall back to the traditional location
    # (or PYAPI_SELECTMODS_DIR) for deployments without the copy.
    for d in (HERE, os.environ.get("PYAPI_SELECTMODS_DIR", "/boot/home")):
        if d not in sys.path:
            sys.path.insert(0, d)
    import selectmods
    found, deps, order, stock, cls_map = selectmods.analyze()
    resolved = []
    for w in wanted:
        if w in found:
            resolved.append(w)
        elif w in cls_map:
            resolved.append(cls_map[w])
    got, stack = set(), list(resolved)
    while stack:
        m = stack.pop()
        if m in got:
            continue
        got.add(m)
        stack.extend(deps.get(m, ()))
    return sorted(got, key=lambda m: order.get(m, 9999))


# --------------------------------------------------------------------------
# Runtime awareness: the TSS-key import wall.
#
# WHY this exists: every imported pybind11 v3 extension module consumes a
# handful of CPython TSS (pthread) keys (its internals managers; measured
# ~4 steady-state, 8-10 for the first module). The pool is process-wide:
#   - Haiku CPython 3.14 (stub TLS): ~507 usable  -> ~120 modules/process
#   - Linux CPython (native pthread): ~1024       -> ~250 modules/process
# Past the pool, the next module's import ABORTS the process ("could not
# initialize the TSS key"). A second, subtler wall: a FAILED import leaves
# partial C++ registrations behind, so the same process can never cleanly
# re-import that module (or anything sharing its types) — import faults are
# process-poisoning.
#
# The suite is therefore subprocess-based (never the whole stack in one
# process), and import_strategy() picks the granularity from the MEASURED
# pool — no platform name-checks: if the module set fits the pool with
# margin, one child process imports everything (fast); otherwise it falls
# back to one fault-isolated child per module (safe). If Haiku's pool ever
# grows (e.g. the deferred lazy-TSS pybind11 patch), this code speeds up
# automatically with no change.
KEYS_PER_MODULE = 4
POOL_MARGIN = 64


def tss_pool():
    """Measure the process TSS-key pool: create keys until failure, release
    them all. None if unmeasurable (stay conservative)."""
    try:
        import ctypes
        lib = ctypes.pythonapi
        lib.PyThread_tss_create.restype = ctypes.c_int
        lib.PyThread_tss_create.argtypes = [ctypes.POINTER(ctypes.c_void_p)]
        lib.PyThread_tss_delete.restype = ctypes.c_int
        lib.PyThread_tss_delete.argtypes = [ctypes.POINTER(ctypes.c_void_p)]
        keys = []
        while len(keys) < 8192:
            k = ctypes.c_void_p()
            if lib.PyThread_tss_create(ctypes.byref(k)) != 0:
                break
            keys.append(k)
        for k in keys:
            lib.PyThread_tss_delete(ctypes.byref(k))
        return len(keys)
    except BaseException:
        return None


def import_strategy(modules):
    """'single' if `modules` fit one process's TSS pool with margin, else
    'per-module' (one fault-isolated child each)."""
    pool = tss_pool()
    if pool is not None and pool >= len(modules) * KEYS_PER_MODULE + POOL_MARGIN:
        return "single"
    return "per-module"


def check(name, fn):
    """Run one assertion group; record {test, status, detail}. A row that
    touches a module that didn't build (an _Absent sentinel) raises Absent →
    recorded ABSENT, so one unbuilt module doesn't take down the rest."""
    try:
        detail = fn()
        RESULTS.append({"test": name, "status": "PASS", "detail": detail or ""})
    except Absent as e:
        absent(name, "not built: %s" % e.name)
    except AssertionError as e:
        RESULTS.append({"test": name, "status": "FAIL", "detail": str(e)[:300]})
    except BaseException as e:
        RESULTS.append({"test": name, "status": "ERROR",
                        "detail": "%s: %s" % (type(e).__name__, e)[:300]})


def skip(name, why):
    RESULTS.append({"test": name, "status": "SKIP", "detail": why})


def report(suite):
    for r in RESULTS:
        r = dict(r)
        r["detail"] = " ".join(str(r.get("detail", "")).split())  # single-line
        print("JSON " + json.dumps({"suite": suite, **r}))
    c = lambda s: sum(1 for r in RESULTS if r["status"] == s)
    print("SUMMARY %s: %d pass, %d fail, %d error, %d absent, %d skip" %
          (suite, c("PASS"), c("FAIL"), c("ERROR"), c("ABSENT"), c("SKIP")))
    return 1 if (c("FAIL") or c("ERROR")) else 0


def scratch_file(name, data=b""):
    p = os.path.join(SCRATCH, name)
    with open(p, "wb") as f:
        f.write(data)
    return p
