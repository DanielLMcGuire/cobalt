from __future__ import annotations

import os
import re

from . import state
from .state import C
from .util import out

def MP(*args):
    return ["makepkg", "--config", state.MAKEPKG_CONF, *args]

def write_makepkg_conf():
    for d in (state.PKGDEST, state.SRCDEST, state.BUILDDIR, state.LOGDIR):
        d.mkdir(parents=True, exist_ok=True)
    state.MAKEPKG_CONF.write_text(
        "source /etc/makepkg.conf\n"
        f'PKGDEST="{state.PKGDEST}"\nSRCDEST="{state.SRCDEST}"\nBUILDDIR="{state.BUILDDIR}"\n'
        f'MAKEFLAGS="-j{C.jobs}"\n'
        'OPTIONS=("${OPTIONS[@]/#debug/!debug}")\n')

def packagelist(d):
    o = out(MP("--packagelist"), cwd=d)
    return [l for l in (o or "").splitlines() if l]

def refresh_sums(d):
    new = out(MP("-g"), cwd=d)
    if not new or not new.strip():
        return False
    start = re.compile(r"^\s*(md5|sha1|sha224|sha256|sha384|sha512|b2|ck)sums(_[A-Za-z0-9_]+)?=\(")
    kept, skip = [], False
    for line in (d / "PKGBUILD").read_text().splitlines():
        if start.match(line):
            skip = True
        if skip:
            if ")" in line:
                skip = False
            continue
        kept.append(line)
    (d / "PKGBUILD").write_text("\n".join(kept) + "\n\n" + new.rstrip("\n") + "\n")
    return True

def pkgfile_name(f):
    return os.path.basename(f).rsplit("-", 3)[0]
