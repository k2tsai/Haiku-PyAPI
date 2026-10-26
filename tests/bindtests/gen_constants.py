#!/usr/bin/env python3
# gen_constants.py — HOST-side generator for test_constants.py expectations.
#
# Every `m.attr("NAME") = <literal>` in the Haiku-PyAPI bindings is checked
# against the beta6 headers (the C library IS the spec). Emits a table of
# (module, name, bound_literal, real_value, verdict) for the test file.
#
#   python3 gen_constants.py <bindings-dir> <headers-dir> <out-test.py>
#
# Resolution: each binding file resolves a name against the enums/macros of
# the headers it #includes first, then against the global (non-compat)
# tables. Macros expand recursively; enums take explicit int/0x/'4cc' values
# or sequential numbering.
#
import os
import re
import sys

MACRO_RE = re.compile(r"^\s*#\s*define\s+([A-Za-z_]\w*)\s+(.*?)\s*(?://[^\n]*)?$")
ATTR_RE = re.compile(
    r'm\.attr\("([A-Za-z_]\w*)"\)\s*=\s*(-?0[xX][0-9a-fA-F]+|-?\d+|\'[^\']{1,4}\'|"[^"]{1,4}")\s*;')
INCLUDE_RE = re.compile(r"#\s*include\s*<([^>]+)>")
SAFE_EVAL_RE = re.compile(r"^[0-9a-fA-FxX \+\-\*\/\|&<>()~^,]*$")


def fourcc(s):
    v = 0
    for c in s[:4]:
        v = (v << 8) | ord(c)
    return v


def read_headers(headers):
    """-> {path: source} for all non-compat .h files"""
    out = {}
    for root, _, files in os.walk(headers):
        if "compatibility" in root or "generated" in root:
            continue
        for fn in files:
            if fn.endswith(".h"):
                p = os.path.join(root, fn)
                try:
                    out[p] = open(p, errors="replace").read()
                except OSError:
                    pass
    return out


def strip_comments(src):
    src = re.sub(r"/\*.*?\*/", "", src, flags=re.S)
    return re.sub(r"//[^\n]*", "", src)


def parse_enum_block(block, macros, out):
    val = None
    for part in block.split(","):
        part = part.strip()
        if not part or part.startswith("typedef") or part.startswith("*"):
            continue
        pm = re.match(r"(\w+)\s*=\s*(.+)$", part)
        if pm:
            name, expr = pm.group(1), pm.group(2).strip()
            val = parse_c_value(expr, macros, out)
        else:
            name = part
            val = 0 if val is None else val + 1
        if val is not None and name not in out:
            out[name] = val


def collect_tables(hdrs):
    global_macros, global_enums = {}, {}
    file_macros, file_enums = {}, {}
    for path, src in hdrs.items():
        fm, fe = {}, {}
        for line in strip_comments(src).splitlines():
            m = MACRO_RE.match(line)
            if m:
                fm[m.group(1)] = m.group(2)
                # skip fssh alias macros (B_X -> FSSH_X) shadowing canonicals
                if not m.group(2).startswith("FSSH_"):
                    global_macros.setdefault(m.group(1), m.group(2))
        clean = strip_comments(src)
        for m in re.finditer(r"enum\s*(?:\w+\s*)?\(([^;{)]*)\)\s*\{|enum\s*\{", clean):
            start, depth, i = m.end(), 1, m.end()
            while i < len(clean) and depth:
                if clean[i] == "{":
                    depth += 1
                elif clean[i] == "}":
                    depth -= 1
                i += 1
            parse_enum_block(clean[start:i - 1], fm, fe)
        for k, v in fe.items():
            global_enums.setdefault(k, v)
        file_macros[path] = fm
        file_enums[path] = fe
    return global_macros, global_enums, file_macros, file_enums


def parse_c_value(expr, macros, enums, depth=0):
    if depth > 8:
        return None
    e = expr.strip().rstrip(";")
    if e.startswith("'") and e.endswith("'") and len(e) >= 2:
        inner = e[1:-1]
        return fourcc(inner) if len(inner) > 1 else ord(inner)
    mm = re.fullmatch(r"([A-Za-z_]\w*)", e)
    if mm:
        key = mm.group(1)
        if key in macros:
            return parse_c_value(macros[key], macros, enums, depth + 1)
        if key in enums:
            return enums[key]
        return None
    if not SAFE_EVAL_RE.match(e):
        return None
    e2 = re.sub(r"[uUlL]+$", "", e.strip())
    if e2 in ("~0", "0xFFFFFFFF", "0xffffffff", "0xFFFFFFFFLL", "-1"):
        pass
    if e2.startswith("~"):
        try:
            return ~int(e2[1:].strip().replace(" ", ""), 0)
        except ValueError:
            return None
    try:
        return int(e2.replace(" ", ""), 0)
    except ValueError:
        return None


def resolve(name, include_paths, file_macros, file_enums,
            global_macros, global_enums):
    # 1) headers this binding includes (by basename match)
    inc = set(include_paths)
    for hp in file_macros:
        if os.path.basename(hp) in inc:
            v = parse_c_value(name, file_macros[hp], file_enums.get(hp, {}))
            if v is not None:
                return v
    # 2) global tables
    v = parse_c_value(name, global_macros, global_enums)
    if v is not None:
        return v
    return None


def main():
    # Optional args, repo-relative defaults (this file lives in
    # <repo>/tests/bindtests/):
    #   gen_constants.py [bindings-dir] [headers-dir] [out-table.py]
    # The Haiku headers tree cannot be vendored here — pass it (arg 2 or the
    # HAIKU_HEADERS env); the beta6 headers are the spec.
    here = os.path.dirname(os.path.abspath(__file__))
    bindings = os.path.abspath(sys.argv[1] if len(sys.argv) > 1
                               else os.path.join(here, "..", "..", "bindings"))
    headers = sys.argv[2] if len(sys.argv) > 2 else os.environ.get("HAIKU_HEADERS")
    if not headers:
        sys.exit("error: beta6 headers dir required (arg 2 or HAIKU_HEADERS)")
    headers = os.path.abspath(headers)
    outpath = sys.argv[3] if len(sys.argv) > 3 else os.path.join(here,
                                                                 "constants_table.py")
    hdrs = read_headers(headers)
    g_macros, g_enums, f_macros, f_enums = collect_tables(hdrs)

    rows = []
    for root, _, files in os.walk(bindings):
        for fn in sorted(files):
            if not fn.endswith(".cpp"):
                continue
            path = os.path.join(root, fn)
            src = open(path, errors="replace").read()
            mod = re.search(r"PYBIND11_MODULE\((\w+)", src)
            if not mod:
                continue
            includes = INCLUDE_RE.findall(src)
            for m in ATTR_RE.finditer(src):
                name, lit = m.group(1), m.group(2)
                if lit[0] == '"':
                    kind, bound = "str4", lit          # python str — shape defect
                elif lit[0] == "'":
                    kind, bound = "chr4", fourcc(lit[1:-1])   # C char literal — int
                else:
                    kind, bound = "int", int(lit, 0)
                real = resolve(name, includes, f_macros, f_enums, g_macros, g_enums)
                if real is None:
                    verdict = "UNRESOLVED"
                elif kind == "str4":
                    verdict = "STR-FORM"      # str where C has an int 4cc
                elif real != bound:
                    verdict = "DRIFT"
                else:
                    verdict = "ok"
                rows.append((mod.group(1), name, lit, real, verdict, kind))

    n_drift = sum(1 for r in rows if r[4] in ("DRIFT", "STR-FORM"))
    n_unres = sum(1 for r in rows if r[4] == "UNRESOLVED")
    lines = ["# GENERATED by gen_constants.py — do not edit by hand.",
             "# rows: (module, name, bound_literal, real_value, verdict, kind)",
             "# verdict: ok | DRIFT | STR-FORM | UNRESOLVED   (real_value None = unresolved)",
             "CONSTANTS = ["]
    for mod, name, lit, real, verdict, kind in rows:
        lines.append("    (%r, %r, %r, %r, %r, %r)," % (mod, name, lit, real,
                                                        verdict, kind))
    lines += ["]"]
    with open(outpath, "w") as f:
        f.write("\n".join(lines) + "\n")
    print("scanned %d constants: %d defects (drift+str-form), %d unresolved, "
          "%d ok" % (len(rows), n_drift, n_unres, len(rows) - n_drift - n_unres))


if __name__ == "__main__":
    main()
