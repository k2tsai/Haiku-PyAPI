#!/usr/bin/env python3
# child_constants.py — one process per module per MODE (TSS wall, and a
# failed solo import leaves partial C++ registrations behind — the closure
# retry MUST run in a fresh process or re-import hits "already registered").
# Usage:
#   python3 child_constants.py <module> '<json rows>' <solo|closure>
# Prints one JSON line: [[name, status, got], ...]
import importlib
import json
import os
import sys
import types

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import common                # FARM + selectmods config live here

FARM = common.FARM


def try_load(mods):
    pkg = types.ModuleType("Be")
    pkg.__path__ = [FARM]
    sys.modules["Be"] = pkg
    for m in mods:
        mod = importlib.import_module("Be." + m)
        for k, v in vars(mod).items():
            if not k.startswith("_"):
                setattr(pkg, k, v)
    return pkg


def as_int(v):
    # pybind11 enum members compare by .value against plain ints
    return getattr(v, "value", v)


def check_rows(Be, rows):
    res = []
    for name, expected in rows:
        v = getattr(Be, name, None)
        if v is None:
            res.append([name, "MISSING", None])
        elif isinstance(expected, str):
            res.append([name, "OK" if v == expected else "FAIL", v])
        elif isinstance(v, bool) or not isinstance(v, int):
            if hasattr(v, "value"):
                res.append([name, "OK" if as_int(v) == expected else "FAIL",
                            repr(v)[:40]])
            else:
                res.append([name, "FAIL-SHAPE", repr(v)[:40]])
        else:
            res.append([name, "OK" if v == expected else "FAIL", v])
    return res


mode = sys.argv[1]
if mode == "chunk":
    # ONE process imports the WHOLE union closure (only when the TSS pool
    # budget says it fits — see common.import_strategy). A failure at module
    # X poisons the process (partial C++ registrations), so report what was
    # loaded and stop; the parent falls back to per-module children for the
    # remainder.
    union = json.loads(sys.argv[2])
    rows_by_mod = json.loads(sys.argv[3])
    import selectmods
    results, failed_at = {}, None
    for m in union:
        try:
            try_load([m])     # Be package persists across try_load calls
        except BaseException:
            failed_at = m
            break
        results[m] = check_rows(sys.modules["Be"], rows_by_mod[m])
    print("JSON " + json.dumps({"loaded": [m for m in results],
                                "failed_at": failed_at,
                                "results": results}))
    sys.exit(0)

mod = sys.argv[1]
rows = json.loads(sys.argv[2])
if mode == "solo":
    Be = try_load([mod])
else:  # closure
    import selectmods
    found, deps, order, stock, _ = selectmods.analyze()
    got, stack = set(), [mod]
    while stack:
        m = stack.pop()
        if m in got:
            continue
        got.add(m)
        stack.extend(deps.get(m, ()))
    Be = try_load(sorted(got, key=lambda m: order.get(m, 9999)))

print("JSON " + json.dumps(check_rows(Be, rows)))
