# bindtests/ — Haiku-PyAPI functional-equivalence test suite

Test-driven standard for the bindings: **the C library (headers) is the
spec**; every binding must be functionally equivalent. Red rows are the fix
backlog; the suite is what the kit-fix agents (2026-10-25 fan-out) work
against.

> **Version note: this suite was written and validated against Haiku R1
> beta6.** Its expectations track beta6-specific changes — 4cc `type_code`s
> (`B_INT32_TYPE = 'LONG'`), POSIX `O_*` open-mode values, removed APIs
> (pinned ABSENT: `BEntryList::SetTo`, `BFile::Close`, `B_FAILED`, ...),
> `ssize_t` attr-I/O returns, the kernel query grammar. On a beta5 (or
> other) tree, regenerate the constants table against THAT tree's headers
> (`gen_constants.py <bindings> <that-headers>`) and expect the beta6
> behavior pins to need rework; the framework (common/child/run_all/
> selectmods/generators) is version-agnostic by design.

## Files

| File | Side | Role |
|---|---|---|
| `gen_constants.py` | host | scans all `m.attr("NAME") = <literal>` in the bindings, resolves NAME against the beta6 headers (macro expansion + enum/4cc parsing, include-aware), emits a verdict table. |
| `gen_test_constants.py` | host | table + manual triage overrides → generates `test_constants.py`. |
| `test_constants.py` | guest | GENERATED. Every bound constant must equal the header value AND be an int where C has an int (string-form 4ccs are defects). Runs under the runtime-aware TSS strategy (see below). |
| `child_constants.py` | guest | the per-module child for the above. |
| `test_storage.py` | guest | runtime standard for the storage kit: BFile (create/Write/Read/Seek/GetSize/Close), attr round-trips per type with ssize_t semantics, GetAttrInfo/GetNextAttrName tuples, BEntry/BPath, BQuery SetPredicate grammar + the "unindexed attr matches nothing" kernel rule + Push*-known-broken. |
| `test_message.py` | guest | pins the messaging core: what-4cc, Add*/Find* (status,value) tuples, type-code 4cc values; RED targets: Flatten/Unflatten safe forms (only raw out-param overloads exist). |
| `test_support.py` | guest | (added by the support agent) BString/BList/BStringList/BAffineTransform/parsedate runtime smoke. |
| `common.py` | guest | loader (farm + selectmods closure), `check`/`skip`/`report` JSON-line protocol, scratch dir. |
| `run_all.py` | guest | runs every `test_*.py`, aggregates, exits nonzero on red. |

## Two tiers: generate (any host) vs execute (where the .so's live)

- **Generation tier** — `gen_constants.py` + `gen_test_constants.py` are
  pure static source analysis (bindings tree + beta6 headers, passed as
  argv). They run anywhere (Linux host or Haiku), import no `Be`, and emit
  `test_constants.py`. Re-run after the bindings source changes.
- **Execution tier** — `test_*.py` + `common.py` + `child_constants.py`
  import the bindings from `BE_PKG_DIR`, so they must run where the `.so`s
  are loadable (the Haiku guest for these builds). The suite dir is
  relocatable: copy it anywhere on that machine, point the env vars (or a
  local `.env`) at the package/bindings.

## Runtime awareness: the TSS-key import wall

The execution tier is subprocess-based because of two process-level walls,
both documented in `common.py`:

1. **TSS pool**: each imported pybind11 v3 module consumes ~4 CPython TSS
   (pthread) keys; the pool is process-wide (~507 on Haiku CPython 3.14,
   ~1024 on Linux). Past the pool the next import aborts the process.
2. **Import poisoning**: a *failed* import leaves partial C++ registrations
   behind, so that process can never cleanly re-import the module — import
   faults need a fresh process.

`common.import_strategy()` **measures the pool at runtime** (create-until-
fail, release all — no platform name-checks) and picks:

- `single` — the whole union closure fits the pool with margin → one child
  process imports everything (fast; e.g. Linux, or a future Haiku build with
  the deferred lazy-TSS pybind11 patch, which this adapts to automatically);
- `per-module` — one fault-isolated child per module, solo→closure retry
  (current Haiku behavior).

`test_constants.py` prints `strategy: ...` on every run; a `single`-mode
child that fails at module X falls back to per-module for X and the rest.

## Configuration

The suite is self-contained and relocatable: copy the directory anywhere
(suites run with `cwd=/` so a local `Be` package can't shadow the farm).
Every path is env-overridable; defaults match the standard installed-VM
layout. `.env` support is stdlib-only (the guest has no pip/python-dotenv),
loaded by `common.py` with dotenv-standard precedence — **real environment
variables always win over `.env`**. Copy `.env.example` → `.env` to customize
(`.env` is git-ignored).

| Variable | Default | Meaning |
|---|---|---|
| `BE_PKG_DIR` | `/boot/home/prefixrun/Be` (else `/boot/home/Be`) | the Be package dir (the .so farm) the tests import |
| `PYAPI_BINDINGS` | `/boot/home/Haiku-PyAPI/bindings` | bindings source tree for `selectmods` static analysis (host dev copy works too) |
| `PYAPI_STOCK_INIT` | `$PYAPI_BINDINGS/__init__.py` | the stock import-order file |
| `PYAPI_BUDGET` | `/boot/home/budget.py` | TSS budget checker for `selectmods.py --budget` (skipped if absent) |
| `PYAPI_SELECTMODS_DIR` | `/boot/home` | fallback dir for `selectmods.py` when not copied alongside the suite |
| `BINDTESTS_SCRATCH` | `<suite>/scratch` | scratch dir for test-created files |
| `BINDTESTS_ENV` | — | alternate `.env` location |

Paths appearing *inside* test assertions (e.g. `BUrl("file:///boot/home")`)
are test DATA, not plumbing — `/boot/home` is universal on Haiku.

## Guest layout

- Suite lives at `/boot/home/pyservices/bindtests/` (deploy via
  `~/nfs-share` → single-file `cp`, md5-verify — never scp, never NFS
  traversal).
- Tests import from the **symlink farm** `/boot/home/prefixrun/Be` →
  `/boot/home/Haiku-PyAPI/build/python3.14_release/` (shared build dir —
  a kit agent's pushed `.so` is visible to every suite run immediately).
- Each kit agent builds in its OWN clone (`/boot/home/hpy-{storage,appmedia,
  iface,support}`) to avoid jam collisions, then pushes rebuilt `.so`s to
  the shared build dir + `/boot/home/Be/` (verify ELF magic `7f454c46`).

## THE RULE: every binding fix ships with a paired regression test

Owner directive (2026-10-25), the pattern all contributors follow:

1. **Name** the row `<kit>:<Class>.<method>` (constants: `constants:<Module>`).
2. **Pin the exact standard shape** — the tuple layout, return convention
   (ssize_t bytes vs status, `(value, status)` order), int-vs-str for 4ccs.
3. **It must have failed before the fix** — exercise the actual bug (the
   no-arg form of an out-param method, the single-arg constructor, the
   constant's type). A test that passes on broken code pins nothing.
4. **Headless-safe**: 1-3 assertions; guard live-resource needs (BApplication,
   screen, audio) with `try: ... except BaseException: common.skip(...)`.
5. **New defect class → new named row** even if you don't fix the code
   (e.g. `query:pushstack-known-broken` pins the upstream QueryStack grammar
   mismatch so a behavior change is visible; `storage:BPath.single-arg-setto`
   pins the `py::arg("leaf")=NULL`-default defect class found 2026-10-25).
6. Kit files: `test_storage.py`, `test_message.py`, `test_support.py`,
   `test_interface.py`, `test_appmedia.py` — `run_all.py` picks up any
   `test_*.py` automatically. Deploy via `~/nfs-share` → single-file `cp`.

Worked example (the standard in one row):

```python
def t_entry_getsize():
    # STANDARD: GetSize() with no args -> (status, size). The pre-fix raw
    # binding was BStatable::GetSize(off_t*) — None null-deref'd, int TypeErrored.
    e = BEntry(P)
    r = e.GetSize()
    assert isinstance(r, tuple) and r[0] == B_OK and r[1] > 0, repr(r)
common.check("storage:entry-getsizeshape", t_entry_getsize)
```

## Regenerating the constants table

```
python3 gen_constants.py ~/src/haiku-pyapi-dev/bindings ~/src/haiku/headers /tmp/constants_table.py
python3 gen_test_constants.py /tmp/constants_table.py > test_constants.py
```
Triage overrides live in `gen_test_constants.py::OVERRIDES/SKIP` (resolver
collisions + hand-verified values, cited to header lines).

## Established conventions the suite pins (so fixes don't regress)

- `BNode::WriteAttr/ReadAttr` → `ssize_t` bytes (negative = error), NOT status.
- `ReadAttr` returns values by type: ints→int, floats→float, bool→bool,
  strings→str, raw/other→bytes; always `(value, bytes_read)`.
- `GetAttrInfo(name)` → `(attr_info, status)`; `attr_info = {type, size}`.
- `GetNextAttrName()` → `(str, status)`; `ReadAttrString(name)` → `(str, status)`.
- type_code is a 4cc on beta6 (`B_INT32_TYPE='LONG'` etc.); open modes are
  POSIX O_* (`B_CREATE_FILE=0x200`).
- BQuery: `SetPredicate` with C-style grammar works; the Push* stack is
  broken against the beta6 kernel (known-broken row, do not "fix" in the
  bindings — the QueryStack grammar is upstream Haiku kit code).
- An attribute predicate matches NOTHING without its volume index (kernel
  rule); `Index::Update` no-ops same-value rewrites → remove-then-write to
  populate.
