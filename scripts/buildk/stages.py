from __future__ import annotations

import os
import re
import shutil
import subprocess
import time
from pathlib import Path

from . import state
from .alpm_hooks import run_install_scripts
from .build import build_base
from .check import check_overlay
from .index import INDEX_TTL, R_BASE, build_index, force_update, index_cache_age, load_index, sync_dbs_present, tok_of
from .install import configure_overlay, extract_from_plan, install_pacstrap, strip_docs
from .kernel import build_kernel
from .plan import write_plan
from .resolve import process_queue
from .state import BASES, C, FAILED, LOCAL_NAMES, MISSING, SEEN_NAME
from .util import die, env, fmt_elapsed, out, rm_children, sh, step, sudo_clear, warn
from .xxc import build_xxc

def build_stage():
    print(f"Kernel: {C.kver} + Packages: [{' '.join(C.pkg_args)}]")
    if C.local_pkgs:
        print(f"Local packages: [{' '.join(os.path.basename(f) for f in C.local_pkgs)}]")

    step("[0/5] Syncing package databases")
    t0 = time.monotonic()
    age = index_cache_age()
    if age is not None and age < INDEX_TTL and sync_dbs_present() and not force_update():
        print(f"  using cached package index ({int(age // 60)} min old, "
              f"TTL {INDEX_TTL // 60} min; FORCE_UPDATE=1 to refresh)")
        load_index((state.IDX / "repo.tsv").read_text(errors="replace"))
    else:
        if sh(["sudo", "pacman", "-Syu", "--noconfirm"], quiet=True) != 0:
            die("pacman -Syu failed")
        build_index()
    print(f"  indexed {len(R_BASE)} official packages")
    print(f"  Updated packages in {fmt_elapsed(time.monotonic() - t0)}.")

    om = Path(C.out_dir)
    reuse = bool(env("SKIP_KERNEL")) and (om / "vmlinuz").is_file() and (om / "modules/version.txt").is_file()
    rm_children(om, keep=("persist", "vmlinuz", "modules") if reuse else ("persist",))
    (om / "modules").mkdir(parents=True, exist_ok=True)
    (om / "persist").mkdir(parents=True, exist_ok=True)

    if reuse:
        step(f"[1/5] Kernel: reusing existing {om}/vmlinuz (SKIP_KERNEL=1)")
        C.krel = (om / "modules/version.txt").read_text().strip()
    else:
        build_kernel()

    step("[2/5] Resolving packages")
    initial = []
    for f in C.local_pkgs:
        p = subprocess.run(["bsdtar", "-xOf", f, ".PKGINFO"], capture_output=True, text=True, errors="replace")
        if p.returncode != 0:
            die(f"'{f}' is not a valid pacman package")
        info = p.stdout
        if not re.search(r"^arch = (x86_64|any)$", info, re.M):
            warn(f"{os.path.basename(f)} is not x86_64/any")
        for m in re.finditer(r"^pkgname = (.*)$", info, re.M):
            LOCAL_NAMES.add(m.group(1))
        for m in re.finditer(r"^provides = (.*)$", info, re.M):
            LOCAL_NAMES.add(tok_of(m.group(1)))
        for m in re.finditer(r"^depend = (.*)$", info, re.M):
            initial.append((m.group(1), os.path.basename(f)))
    initial += [(f"arch:{d}", "(cobalt-base)") for d in ("filesystem", "glibc", "bash", "busybox")]
    initial += [(d, "(requested)") for d in C.pkg_args]
    process_queue_t0 = time.monotonic()
    process_queue(initial)
    print(f"  Resolved {len(SEEN_NAME)} packages in "
          f"{fmt_elapsed(time.monotonic() - process_queue_t0)}.")

    if MISSING and not env("KEEP_GOING"):
        die(f"unresolved packages: {' '.join(MISSING)} (KEEP_GOING=1 to continue anyway)")

    step(f"[3/5] Building {len(BASES)} package(s) from source")
    for b in BASES:
        build_base(b)
    if FAILED:
        warn(f"failed packages: {' '.join(FAILED)}")
    if not C.no_xxc:
        build_xxc()
    write_plan()
    print(f"  build plan: {state.CACHE}/plan.tsv")

def install_stage():
    mode = state.INSTALL_MODE
    if not (state.CACHE / "plan.tsv").is_file():
        die(f"no build plan at {state.CACHE}/plan.tsv (the build stage has not run)")
    om = Path(C.out_dir)

    step(f"[4/5] Installing into overlay ({state.TARGET_OVERLAY}) [mode: {mode}]")
    if os.path.isdir(state.TARGET_OVERLAY):
        sudo_clear(state.TARGET_OVERLAY)
    Path(state.TARGET_OVERLAY).mkdir(parents=True, exist_ok=True)
    (om / "persist").mkdir(parents=True, exist_ok=True)

    if mode == "pacstrap":
        if not install_pacstrap():
            if env("INSTALL_STRICT"):
                die(f"pacstrap install failed and INSTALL_STRICT=1 (log: {state.LOGDIR}/pacstrap.log)")
            warn(f"pacstrap failed (log: {state.LOGDIR}/pacstrap.log)")
            warn("falling back to plain extraction: package install scripts and hooks will NOT run")
            sudo_clear(state.TARGET_OVERLAY)
            mode = "extract"
    if mode == "extract":
        extract_from_plan()

    manifest = [l for l in (state.CACHE / "plan.tsv").read_text().splitlines() if l and not l.startswith("#")]
    (om / "manifest.tsv").write_text("".join("\t".join(l.split("\t")[:5]) + "\n" for l in manifest))

    if mode == "pacstrap":
        if not env("KEEP_PACMAN_DB"):
            print("  removing pacman database from the image (KEEP_PACMAN_DB=1 to keep it)")
            sh(["sudo", "rm", "-rf", f"{state.TARGET_OVERLAY}/var/lib/pacman"])
        sh(["sudo", "rm", "-rf", f"{state.TARGET_OVERLAY}/var/cache/pacman", f"{state.TARGET_OVERLAY}/var/log/pacman.log"])
    sh(["sudo", "chown", "-R", f"{os.getuid()}:{os.getgid()}", state.TARGET_OVERLAY])

    configure_overlay()

    if mode == "extract" and env("RUN_SCRIPTS", "1") != "0":
        failed = run_install_scripts()
        (state.TMP / "postinst.txt").write_text("".join(f + "\n" for f in failed))
        sh(["sudo", "chown", "-R", f"{os.getuid()}:{os.getgid()}", state.TARGET_OVERLAY])

    strip_docs()

    skipped = state.CACHE / "skipped.tsv"
    if skipped.is_file() and skipped.stat().st_size > 0:
        shutil.copy2(skipped, om / "skipped.txt")
        print(f"\nSkipped (closed-source / prebuilt; list in {om}/skipped.txt):")
        for l in (om / "skipped.txt").read_text().splitlines():
            print(f"  {l}")
    else:
        (om / "skipped.txt").write_text("")

    step("[5/5] Checking overlay")
    check_overlay(state.TARGET_OVERLAY)

    size = (out(["du", "-sh", state.TARGET_OVERLAY]) or "?").split()[0]
    print(f"\nInstalled via: {mode}")
    print(f"Overlay uncompressed size: {size}")
    print(f"Manifest: {om}/manifest.tsv")
    print("Build command:")
    print(f"  make -f Makefile.uefi cobalt KERNEL={om}/vmlinuz MODULES={om}/modules "
          f"OVERLAY={om}/overlay BUSYBOX={om}/overlay/usr/bin/busybox")
