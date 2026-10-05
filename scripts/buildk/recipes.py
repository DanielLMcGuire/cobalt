from __future__ import annotations

import os
import shutil
import time

from . import state
from .state import CloneError, TagMissing
from .util import out, sh, tail, warn

def clone_pkg(src, base, ref=""):
    d = state.WORK / "pkg" / src / base
    clog = state.LOGDIR / f"clone-{src}-{base}.log"
    genv = {**os.environ, "GIT_TERMINAL_PROMPT": "0"}
    if not (d / ".git").is_dir():
        d.parent.mkdir(parents=True, exist_ok=True)
        if src == "arch":
            url = f"https://gitlab.archlinux.org/archlinux/packaging/packages/{base.replace('+', 'plus')}.git"
        else:
            url = f"https://aur.archlinux.org/{base}.git"
        clog.write_bytes(b"")
        ok = False
        for attempt in (1, 2, 3):
            shutil.rmtree(d, ignore_errors=True)
            if sh(["git", "clone", "-q", url, d], envv=genv, log=clog, append=True) == 0:
                ok = True
                break
            time.sleep(attempt * 3)
        if not ok and src == "arch" and shutil.which("pkgctl"):
            shutil.rmtree(d, ignore_errors=True)
            if sh(["pkgctl", "repo", "clone", "--protocol=https", base], cwd=d.parent,
                  envv=genv, log=clog, append=True) == 0 and (d.parent / base).exists():
                os.rename(d.parent / base, d)
                ok = True
        if not ok:
            lines = "\n".join("    git: " + l for l in tail(clog, 6))
            raise CloneError(f"clone failed: {url}\n{lines}")
    if ref and sh(["git", "-C", d, "checkout", "-q", "-f", ref], quiet_all=True) != 0:
        raise TagMissing(ref)
    return d

KEYSERVERS = ("hkps://keyserver.ubuntu.com", "hkps://keys.openpgp.org",
              "hkps://pgp.mit.edu", "hkp://keyserver.ubuntu.com:80")

def import_keys(d):
    o = out(["bash", "-c", 'source ./PKGBUILD >/dev/null 2>&1; printf "%s\\n" "${validpgpkeys[@]}"'], cwd=d)
    for k in (o or "").split():
        if sh(["gpg", "--list-keys", k], quiet_all=True) == 0:
            continue
        glog = state.LOGDIR / f"gpg-{k}.log"
        glog.write_bytes(b"")
        for ks in KEYSERVERS:
            sh(["timeout", "30", "gpg", "--keyserver", ks, "--recv-keys", k], log=glog, append=True)
            if sh(["gpg", "--list-keys", k], quiet_all=True) == 0:
                break
        else:
            warn(f"could not fetch PGP key {k} from any keyserver (details: {glog}); "
                 "the signature check will fail (SKIP_PGP=1 skips it)")
