from __future__ import annotations

import glob
import os
import shutil
import sys
from pathlib import Path

from . import state
from .plan import read_plan
from .util import die, env, out, sh, tail, warn

def install_pacstrap():
    repo, conf = state.CACHE / "repo", state.CACHE / "pacman-cobalt.conf"
    for tool, pkg in (("pacstrap", "arch-install-scripts"), ("repo-add", None)):
        if not shutil.which(tool):
            warn(f"pacstrap not installed (package {pkg})" if pkg else "repo-add not found")
            return False
    shutil.rmtree(repo, ignore_errors=True)
    repo.mkdir(parents=True)
    (state.CACHE / "pacman-cache").mkdir(parents=True, exist_ok=True)
    names = []
    for name, _ver, _src, _base, _commit, f in read_plan():
        names.append(name)
        link = repo / os.path.basename(f)
        link.unlink(missing_ok=True)
        link.symlink_to(f)
    pkgs = sorted(glob.glob(str(repo / "*.pkg.tar.*")))
    if not pkgs or sh(["repo-add", "-q", "cobalt.db.tar.zst", *[os.path.basename(p) for p in pkgs]],
                      cwd=repo, quiet_all=True) != 0:
        warn("repo-add failed")
        return False
    conf.write_text(f"[options]\nArchitecture = x86_64\nCacheDir = {state.CACHE}/pacman-cache\n"
                    f"SigLevel = Never\nLocalFileSigLevel = Never\n\n[cobalt]\nServer = file://{repo}\n")
    print(f"  pacstrap: installing {len(names)} package(s) from the local repo (install scripts + hooks run)")
    plog = state.LOGDIR / "pacstrap.log"
    if sh(["sudo", "pacstrap", "-C", conf, "-G", "-M", state.TARGET_OVERLAY, *names], log=plog) != 0:
        print("\n".join(tail(plog, 25)), file=sys.stderr)
        return False
    return True

def extract_pkg(pkg):
    rc = sh(["sudo", "bsdtar", "-xpf", pkg, "-C", state.TARGET_OVERLAY,
             "--exclude", ".PKGINFO", "--exclude", ".BUILDINFO", "--exclude", ".MTREE",
             "--exclude", ".INSTALL", "--exclude", ".CHANGELOG"])
    if rc != 0:
        die(f"failed to extract {pkg}")

PKG_INFO = {}

def extract_from_plan():
    PKG_INFO.clear()
    post = []
    for name, ver, _s, _b, _c, f in read_plan():
        print(f"      {os.path.basename(f)}")
        extract_pkg(f)
        listing = (out(["bsdtar", "-tf", f]) or "").splitlines()
        has_inst = ".INSTALL" in listing
        PKG_INFO[name] = {"ver": ver, "file": f, "install": has_inst,
                          "paths": [l for l in listing if l and not l.startswith(".")]}
        if has_inst:
            post.append(name)
    (state.TMP / "postinst.txt").write_text("".join(n + "\n" for n in post))

def configure_overlay():
    ov = Path(state.TARGET_OVERLAY)
    for link, target in (("bin", "usr/bin"), ("sbin", "usr/bin"), ("lib", "usr/lib"),
                         ("lib64", "usr/lib"), ("usr/sbin", "bin")):
        p = ov / link
        if not p.exists() and not p.is_symlink():
            p.parent.mkdir(parents=True, exist_ok=True)
            p.symlink_to(target)
    (ov / "usr/lib").mkdir(parents=True, exist_ok=True)
    (ov / "etc").mkdir(parents=True, exist_ok=True)
    (ov / "etc/ld.so.conf.d").mkdir(parents=True, exist_ok=True)

    (ov / "etc/ld.so.conf").write_text("/usr/lib\ninclude /etc/ld.so.conf.d/*.conf\n")
    (ov / "usr/lib/os-release").write_text(
        'NAME="Cobalt Linux"\nPRETTY_NAME="Cobalt Linux"\nID=cobalt\nID_LIKE=arch\n'
        'BUILD_ID=rolling\nANSI_COLOR="38;2;0;71;171"\n')
    osr = ov / "etc/os-release"
    if osr.is_symlink() or osr.exists():
        osr.unlink()
    osr.symlink_to("../usr/lib/os-release")

    for real, alias in (("gcc", "cc"), ("g++", "c++")):
        if (ov / "usr/bin" / real).exists() and not (ov / "usr/bin" / alias).exists():
            (ov / "usr/bin" / alias).symlink_to(real)

def strip_docs():
    if not env("KEEP_DOCS"):
        print("Stripping docs/man/locales (set KEEP_DOCS=1 to keep)...")
        ov = Path(state.TARGET_OVERLAY)
        for d in ("doc", "man", "info", "gtk-doc", "locale", "bash-completion"):
            shutil.rmtree(ov / "usr/share" / d, ignore_errors=True)

def cleanup():
    if state.TMP is not None:
        shutil.rmtree(state.TMP, ignore_errors=True)
    if state.TARGET_OVERLAY and os.path.isdir(state.TARGET_OVERLAY):
        sh(["sudo", "chown", "-R", f"{os.getuid()}:{os.getgid()}", state.TARGET_OVERLAY], quiet_all=True)
