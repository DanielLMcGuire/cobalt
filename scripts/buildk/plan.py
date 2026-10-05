from __future__ import annotations

import os

from . import state
from .makepkg import packagelist, pkgfile_name
from .state import BASES, BASE_DIR, BASE_SRC, C, SKIPPED, WANT
from .util import out, warn
from .xxc import XXC_PKGS

def write_plan():
    lines = []
    ordered = [b for b in BASES if b == "filesystem"] + [b for b in BASES if b != "filesystem"]
    for base in ordered:
        d = BASE_DIR[base]
        commit = (out(["git", "-C", d, "rev-parse", "--short", "HEAD"]) or "").strip() or "-"
        for f in packagelist(d):
            name = pkgfile_name(f)
            if name not in WANT.get(base, []):
                continue
            if not os.path.isfile(f):
                warn(f"missing build output for {name} ({f})")
                continue
            ver = os.path.basename(f)[len(name) + 1:].rsplit("-", 1)[0]
            lines.append("\t".join([name, ver, BASE_SRC[base], base, commit, f]))
    for f in XXC_PKGS:
        name = pkgfile_name(f)
        ver = os.path.basename(f)[len(name) + 1:].rsplit("-", 1)[0]
        lines.append("\t".join([name, ver, "local", "libxxc", "-", f]))
    for f in C.local_pkgs:
        lines.append("\t".join([pkgfile_name(f), "-", "local", "-", "-", f]))
    (state.CACHE / "plan.tsv").write_text("\n".join(lines) + "\n")
    (state.CACHE / "skipped.tsv").write_text("".join(f"{n}\t{r}\n" for n, r in SKIPPED.items()))

def read_plan():
    rows = []
    for line in (state.CACHE / "plan.tsv").read_text().splitlines():
        if not line or line.startswith("#"):
            continue
        f = line.split("\t")
        f += [""] * (6 - len(f))
        rows.append(f[:6])
    return rows
