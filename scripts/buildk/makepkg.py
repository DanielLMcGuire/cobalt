from __future__ import annotations

import os
import re
import shlex

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

def _parse_pkgbuild_list(value):
    value = value.strip()
    if value.startswith("(") and value.endswith(")"):
        value = value[1:-1]
    return shlex.split(value, posix=True)


def _fallback_packagelist(d):
    try:
        text = (d / "PKGBUILD").read_text(encoding="utf-8", errors="replace")
    except OSError:
        return []

    match = re.search(r"^\s*pkgname\s*=\s*(.+)$", text, re.MULTILINE)
    if not match:
        return []
    pkgname = _parse_pkgbuild_list(match.group(1))

    match = re.search(r"^\s*pkgver\s*=\s*(.+)$", text, re.MULTILINE)
    if not match:
        return []
    pkgver = match.group(1).strip().strip("'\"")

    match = re.search(r"^\s*pkgrel\s*=\s*(.+)$", text, re.MULTILINE)
    if not match:
        return []
    pkgrel = match.group(1).strip().strip("'\"")

    arch = "any"
    match = re.search(r"^\s*arch\s*=\s*(.+)$", text, re.MULTILINE)
    if match:
        arch_names = _parse_pkgbuild_list(match.group(1))
        if arch_names:
            arch = arch_names[0]

    if not pkgname or not pkgver or not pkgrel:
        return []

    pkgdest = state.PKGDEST
    if pkgdest is None:
        return []

    return [str(pkgdest / f"{name}-{pkgver}-{pkgrel}-{arch}.pkg.tar.zst") for name in pkgname]


def packagelist(d):
    o = out(MP("--packagelist"), cwd=d)
    files = [l for l in (o or "").splitlines() if l]
    if files:
        return files
    return _fallback_packagelist(d)


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
