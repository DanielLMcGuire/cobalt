from __future__ import annotations

import hashlib
import os
import sys

from . import state
from .makepkg import MP, packagelist, pkgfile_name
from .state import HERE
from .util import die, env, sh, step, tail

XXC_PKGS = []

def xxc_source_hash():
    h = hashlib.sha256()
    for rel in ("PKGBUILD", "LICENSE", "CMakeLists.txt", "cmake", "platform", "libminicrt", "libxxc"):
        p = HERE / rel
        files = [p] if p.is_file() else sorted(q for q in p.rglob("*") if q.is_file())
        for f in files:
            h.update(f.relative_to(HERE).as_posix().encode() + b"\0")
            h.update(f.read_bytes() + b"\0")
    return h.hexdigest()

def build_xxc():
    step("Building ++C (libxxc, libminicrt) from this tree")
    if not (HERE / "PKGBUILD").is_file():
        die(f"no PKGBUILD in {HERE} (use --no-xxc to skip ++C)")
    listed = packagelist(HERE)
    if not listed:
        die("makepkg --packagelist returned nothing for the ++C PKGBUILD")
    stamp, cur = state.CACHE / "xxc.stamp", xxc_source_hash()
    stale = not stamp.is_file() or stamp.read_text().strip() != cur
    need = bool(env("FORCE")) or stale or any(not os.path.isfile(f) for f in listed)
    if need:
        xlog = state.LOGDIR / "xxc.log"
        print(f"  building libxxc 0.1.0 [local]...")
        print(f"    (log: {xlog})", flush=True)
        flags = ["--noconfirm", "--nodeps", "--force", "--nocheck"]
        if sh(MP(*flags), cwd=HERE, log=xlog) != 0:
            print("\n".join(tail(xlog, 30)), file=sys.stderr)
            die(f"++C build failed (log: {xlog})")
        stamp.write_text(cur + "\n")
    else:
        for f in listed:
            print(f"  cached: {pkgfile_name(f)}")
    XXC_PKGS[:] = [f for f in listed if os.path.isfile(f)]
