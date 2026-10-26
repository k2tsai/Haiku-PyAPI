#!/usr/bin/env python3
#
# run_all.py — run every test_*.py in this dir (guest), aggregate the JSON
# lines, print a pass/fail/error table, exit nonzero if anything is red.
#
import glob
import json
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SUITES = sorted(os.path.basename(p)[:-3]
                for p in glob.glob(HERE + "/test_*.py"))

rows = []
for suite in SUITES:
    try:
        p = subprocess.run([sys.executable, os.path.join(HERE, suite + ".py")],
                           cwd="/", capture_output=True, text=True, timeout=900)
    except subprocess.TimeoutExpired as te:
        # A hanging suite must not sink the whole aggregate: record it and
        # move on (other agents own their suites; we only need totals).
        rows.append({"test": suite + ":CRASH",
                     "status": "ERROR",
                     "detail": "timed out after 900s"})
        continue
    n_json = 0
    for line in (p.stdout or "").splitlines():
        if line.startswith("JSON "):
            d = json.loads(line[5:])
            d.pop("suite", None)
            rows.append(d)
            n_json += 1
    if p.returncode != 0 or n_json == 0:
        tail = (p.stderr or p.stdout or "").strip().splitlines()
        rows.append({"test": suite + ":CRASH",
                     "status": "ERROR" if p.returncode != 1 else "FAIL",
                     "detail": (tail[-1][:200] if tail else
                                "no JSON output (rc=%d)" % p.returncode)})

by = {}
for r in rows:
    by.setdefault(r["status"], []).append(r)
for status in ("PASS", "FAIL", "ERROR", "SKIP"):
    for r in by.get(status, []):
        print("%-6s %-52s %s" % (status, r["test"][:52], r.get("detail", "")))
print("==== %d pass, %d fail, %d error, %d skip ====" %
      (len(by.get("PASS", [])), len(by.get("FAIL", [])),
       len(by.get("ERROR", [])), len(by.get("SKIP", []))))
sys.exit(1 if (by.get("FAIL") or by.get("ERROR")) else 0)
