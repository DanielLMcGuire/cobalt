from __future__ import annotations

import os
import shutil
import sys
from pathlib import Path

from .state import C, HERE, IMG
from .util import die, env, sh

def container_prepare():
    for e in ("docker", "podman"):
        if shutil.which(e) and sh([e, "info"], quiet_all=True) == 0:
            C.engine = e
            break
    else:
        die("not on Arch, and no working container engine found.\n"
            "  - WSL + Docker Desktop: enable Settings > Resources > WSL Integration for this distro\n"
            "  - or install podman:    sudo apt install podman\n"
            "  - or run natively inside an Arch distro (wsl --install archlinux)")

    print(f"[*] Not on Arch: building inside a container ({C.engine})", flush=True)
    rc = sh([C.engine, "build", "-q", "-t", IMG,
             "--build-arg", f"UID={os.getuid()}", "--build-arg", f"GID={os.getgid()}",
             "-f", HERE / "Dockerfile.buildk", HERE], quiet=True)
    if rc != 0:
        die("container image build failed")

    cache = Path(env("BUILDK_CACHE") or Path.cwd() / ".buildk-cache")
    Path(C.out_dir).mkdir(parents=True, exist_ok=True)
    cache.mkdir(parents=True, exist_ok=True)
    mounts = {}
    for d in (Path.cwd(), HERE, Path(C.out_dir).resolve(), cache.resolve()):
        mounts[str(d)] = 1
    for f in C.local_pkgs:
        mounts[os.path.dirname(f)] = 1
    for d in mounts:
        C.mount_args += ["-v", f"{d}:{d}"]

def container_run(stage, priv):
    cache = env("BUILDK_CACHE") or str(Path.cwd() / ".buildk-cache")
    args = ["run", "--rm", "-w", str(Path.cwd()),
            "-e", "BUILDK_IN_CONTAINER=1", "-e", f"BUILDK_STAGE={stage}",
            "-e", f"BUILDK_CACHE={cache}"]
    if C.engine == "podman":
        args.append("--userns=keep-id")
    if sys.stdin.isatty():
        args.append("-i")
    if priv:
        args.append("--privileged")
    args += C.mount_args
    for v in ("INSTALL_MODE INSTALL_STRICT KEEP_PACMAN_DB SKIP_KERNEL JOBS KEEP_GOING KEEP_DOCS "
              "ALLOW_BINARY CHECK SKIP_PGP PIN FORCE RESOLVE_JOBS FORCE_UPDATE RUN_SCRIPTS REBUILD").split():
        if v in os.environ:
            args += ["-e", v]
    if C.engine == "podman" and stage == "install":
        args += ["-e", "INSTALL_MODE=extract"]
    return sh([C.engine, *args, IMG, "python3", HERE / "buildk.py", *C.orig_args])