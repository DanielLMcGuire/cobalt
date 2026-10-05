from __future__ import annotations

import hashlib
import os
import subprocess
import sys

from . import state
from .aur import aur_lookup
from .index import repo_resolve, tok_of
from .makepkg import MP, packagelist, pkgfile_name
from .pkghooks import flush_notes
from .recipes import import_keys
from .resolve import prep_base
from .srcinfo import is_closed
from .state import BASE_DIR, BASE_SRC, BUILDING, BUILT, FAILED, HERE, SI, WANT
from .util import die, env, out, sh, tail

def install_build_deps(base):
    si = SI[base]
    own = set(si.names)
    deps = si.build_deps(bool(env("CHECK")))
    if not deps:
        return
    p = subprocess.run(["pacman", "-T", *deps], capture_output=True, text=True, errors="replace")
    repo_need, aur_need = [], []
    for d in p.stdout.splitlines():
        if not d:
            continue
        name = repo_resolve(d)
        if name:
            repo_need.append(name)
        else:
            if tok_of(d) in own:
                continue
            aur_need.append(d)
    if repo_need:
        if sh(["sudo", "pacman", "-S", "--needed", "--noconfirm", "--asdeps", *repo_need], quiet=True) != 0:
            die(f"{base}: failed to install build dependencies: {' '.join(repo_need)}")
    for d in aur_need:
        got = aur_lookup(d)
        if not got:
            die(f"{base}: build dependency '{d}' not found in repos or AUR")
        abase = got[1]
        print(f"  build-only AUR dependency: {abase} (for {base})")
        prep_base("aur", abase, stream=True)
        flush_notes(abase)
        if is_closed(SI[abase], abase) and not env("ALLOW_BINARY"):
            die(f"{base}: build dependency {abase} looks closed-source/prebuilt (ALLOW_BINARY=1 to override)")
        build_base(abase)
        inst = [f for f in packagelist(BASE_DIR[abase]) if os.path.isfile(f)]
        if inst:
            sh(["sudo", "pacman", "-U", "--needed", "--noconfirm", "--asdeps", *inst], quiet=True)

def _hook_hash(base):
    h = HERE / "pkghooks" / f"{base}.sh"
    return hashlib.sha256(h.read_bytes()).hexdigest() if h.is_file() else ""

def build_base(base):
    d = BASE_DIR[base]
    if base in BUILT:
        return
    if base in BUILDING:
        die(f"dependency cycle through {base}")
    BUILDING.add(base)

    names = WANT.get(base)
    need = False
    for f in packagelist(d):
        if not names or pkgfile_name(f) in names:
            if not os.path.isfile(f):
                need = True
    if env("FORCE") or base in (env("REBUILD") or "").replace(",", " ").split():
        need = True

    hh = _hook_hash(base)
    stamp = d / ".buildk-hook-stamp"
    if hh and not need:
        if not stamp.is_file():
            stamp.write_text(hh)
        elif stamp.read_text().strip() != hh:
            print(f"  hook for {base} changed: rebuilding")
            need = True

    if need:
        log = state.LOGDIR / f"{base}.log"
        print(f"  building {base} ({SI[base].pkgver}) [{BASE_SRC[base]}]...")
        print(f"    (log: {log})", flush=True)
        install_build_deps(base)
        import_keys(d)
        flags = ["--noconfirm", "--nodeps", "--force"]
        if not env("CHECK"):
            flags.append("--nocheck")
        if env("SKIP_PGP"):
            flags.append("--skippgpcheck")
        if sh(MP(*flags), cwd=d, log=log) != 0:
            print("\n".join(tail(log, 25)), file=sys.stderr)
            FAILED.append(base)
            print(f"  FAILED: {base} (log: {log})", file=sys.stderr)
            if not env("KEEP_GOING"):
                die(f"build of {base} failed; set KEEP_GOING=1 to build anyway")
        if hh and base not in FAILED:
            stamp.write_text(hh)
    else:
        print(f"  cached: {base}")
    BUILT.add(base)
    BUILDING.discard(base)

def has_install_script(pkg):
    o = out(["bsdtar", "-tf", pkg])
    return bool(o) and ".INSTALL" in o.splitlines()
