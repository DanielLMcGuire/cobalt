from __future__ import annotations

import os
import re
import shutil
import stat
import sys
from concurrent.futures import ThreadPoolExecutor

from . import state
from .state import C
from .util import env, out

def _needed_libs(path):
    o = out(["readelf", "-d", path])
    return re.findall(r"\(NEEDED\).*\[(.*)\]", o or "")

def _is_elf(path):
    try:
        with open(path, "rb") as f:
            return f.read(4) == b"\x7fELF"
    except OSError:
        return False

def _expected_dangling(tgt, docs_stripped):
    if re.match(r"^/(proc|run|dev|sys|tmp)(/|$)", tgt):
        return True
    if re.match(r"^/(usr/)?lib32(/|$)", tgt):
        return True
    return bool(docs_stripped and re.match(r"^/usr/share/(doc|man|info|gtk-doc|locale)(/|$)", tgt))

def check_overlay(ov):
    if not shutil.which("readelf"):
        print("  (readelf not found; skipping library check)")
        return
    ov = str(ov)
    have, elfs, links = set(), [], []
    for root, dirs, files in os.walk(ov):
        for n in dirs + files:
            p = os.path.join(root, n)
            is_link = os.path.islink(p)
            if is_link:
                links.append(p)
            if ".so" in n and (is_link or os.path.isfile(p)):
                have.add(n)
        for n in files:
            p = os.path.join(root, n)
            if os.path.islink(p):
                continue
            try:
                mode = os.stat(p).st_mode
            except OSError:
                continue
            if (mode & (stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH) or ".so" in n) and _is_elf(p):
                elfs.append(p)

    needed_by = {}
    with ThreadPoolExecutor(max_workers=C.jobs) as pool:
        for p, libs in zip(elfs, pool.map(_needed_libs, elfs)):
            for lib in libs:
                needed_by.setdefault(lib, []).append(p[len(ov):])
    missing = sorted(set(needed_by) - have)
    if missing:
        print("  WARNING: libraries required by overlay binaries but not in the overlay:", file=sys.stderr)
        for m in missing:
            users = sorted(needed_by[m])
            more = f" and {len(users) - 3} more" if len(users) > 3 else ""
            print(f"    {m}  (needed by {', '.join(users[:3])}{more})", file=sys.stderr)
    else:
        print("  all shared-library dependencies are satisfied")

    docs_stripped = not env("KEEP_DOCS")
    n = expected = 0
    for l in links:
        rel = l[len(ov):]
        t = os.readlink(l)
        tgt = os.path.normpath(t if t.startswith("/") else os.path.join(os.path.dirname(rel), t))
        if os.path.lexists(ov + tgt):
            continue
        if _expected_dangling(tgt, docs_stripped):
            expected += 1
            continue
        n += 1
        if n <= 15:
            if n == 1:
                print("  WARNING: dangling symlinks:", file=sys.stderr)
            print(f"    {rel} -> {t}", file=sys.stderr)
    if n > 15:
        print(f"    ... and {n - 15} more", file=sys.stderr)
    if expected:
        print(f"  ({expected} expected dangling symlink(s) ignored: runtime paths, stripped docs, 32-bit leftovers)")

    pi = state.TMP / "postinst.txt"
    if pi.exists() and pi.read_text().strip():
        print("  note: install scripts / libalpm hooks that did NOT run or failed:", file=sys.stderr)
        for l in pi.read_text().splitlines():
            print(f"    {l}", file=sys.stderr)
