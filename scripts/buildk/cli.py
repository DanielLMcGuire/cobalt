from __future__ import annotations

import argparse
import atexit
import os
import re
import shutil
import tempfile
from pathlib import Path

from . import state
from .container import container_prepare, container_run
from .install import cleanup
from .makepkg import write_makepkg_conf
from .stages import build_stage, install_stage
from .state import C
from .util import die, env

def parse_args(argv):
    ap = argparse.ArgumentParser(
        prog="buildk.py", description=__doc__, allow_abbrev=False,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--no-xxc", action="store_true")
    ap.add_argument("args", nargs="*")
    ns = ap.parse_intermixed_args(argv)
    C.no_xxc = ns.no_xxc
    pos = ns.args
    if len(pos) > 0:
        C.kver = pos[0]
    if len(pos) > 1:
        C.out_dir = pos[1]
    for a in pos[2:]:
        if ".pkg.tar" in a or "/" in a:
            if not os.path.isfile(a):
                die(f"'{a}' looks like a file path but does not exist")
            C.local_pkgs.append(os.path.abspath(a))
        else:
            C.pkg_args.append(a)

def main(argv):
    C.orig_args = list(argv)
    parse_args(argv)
    state.INSTALL_MODE = env("INSTALL_MODE", "pacstrap")
    if state.INSTALL_MODE not in ("pacstrap", "extract"):
        die("INSTALL_MODE must be 'pacstrap' or 'extract'")

    if re.match(r"^/mnt/[a-z](/|$)", str(Path.cwd())):
        die(f"running from a Windows drive ({Path.cwd()}): it is case-insensitive and the kernel tree "
            "will not build there. Copy the project into the Linux filesystem (e.g. ~/cobalt) and run it from there.")

    if not env("BUILDK_IN_CONTAINER") and (not os.path.exists("/etc/arch-release") or not shutil.which("pacman")):
        container_prepare()
        if state.INSTALL_MODE == "pacstrap":
            rc = container_run("build", False)
            if rc:
                return rc
            return container_run("install", True)
        return container_run("all", False)

    stage = env("BUILDK_STAGE", "all")
    os.environ["PATH"] = os.environ.get("PATH", "") + ":/usr/bin/site_perl:/usr/bin/vendor_perl:/usr/bin/core_perl"
    if os.geteuid() == 0:
        die("do not run as root (makepkg refuses); use the container or a normal user with sudo")
    for t in ("git", "makepkg", "expac", "bsdtar", "cmake", "ninja", "sudo"):
        if not shutil.which(t):
            die(f"missing tool '{t}' (see Dockerfile.buildk for the package list)")

    C.jobs = int(env("JOBS") or os.cpu_count() or 1)
    state.CACHE = Path(env("BUILDK_CACHE") or Path.cwd() / ".buildk-cache")
    state.CACHE.mkdir(parents=True, exist_ok=True)
    state.CACHE = state.CACHE.resolve()
    state.WORK, state.PKGDEST, state.SRCDEST, state.BUILDDIR = state.CACHE / "work", state.CACHE / "pkgs", state.CACHE / "src", state.CACHE / "build"
    state.LOGDIR, state.IDX, state.MAKEPKG_CONF = state.CACHE / "logs", state.CACHE / "index", state.CACHE / "makepkg.conf"
    state.WORK.mkdir(parents=True, exist_ok=True)
    state.LOGDIR.mkdir(parents=True, exist_ok=True)
    write_makepkg_conf()

    Path(C.out_dir).mkdir(parents=True, exist_ok=True)
    C.out_dir = str(Path(C.out_dir).resolve())
    state.TARGET_OVERLAY = os.path.join(C.out_dir, "overlay")
    state.TMP = Path(tempfile.mkdtemp())
    atexit.register(cleanup)

    if stage in ("all", "build"):
        build_stage()
    if stage in ("all", "install"):
        install_stage()
    return 0
