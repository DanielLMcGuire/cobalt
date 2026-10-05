from __future__ import annotations

import hashlib
import os
import shutil
import sys
from pathlib import Path

from . import state
from .index import repo_tag
from .makepkg import MP
from .pkghooks import run_hook
from .recipes import clone_pkg, import_keys
from .state import C, CloneError, HERE, TagMissing
from .util import cp_a, die, env, out, sh, step, tail, warn

def _sha_line(path):
    return f"{hashlib.sha256(Path(path).read_bytes()).hexdigest()}  -\n"

def build_kernel():
    step("[1/5] Kernel from source (Arch 'linux' recipe + Cobalt config)")
    ref = ""
    if C.kver == "latest":
        if env("PIN", "1") != "0":
            ref = repo_tag("linux")
    else:
        ref = C.kver
    try:
        d = clone_pkg("arch", "linux", ref)
    except (CloneError, TagMissing) as e:
        if isinstance(e, CloneError):
            print(f"  {e}", file=sys.stderr)
        if ref and C.kver == "latest":
            warn(f"tag '{ref}' not found in linux packaging repo; using HEAD")
            try:
                d = clone_pkg("arch", "linux")
            except CloneError as e2:
                print(f"  {e2}", file=sys.stderr)
                die("cannot clone Arch linux packaging repo")
        else:
            die(f"cannot clone Arch linux packaging repo at '{ref or 'HEAD'}'")
    run_hook("linux", d, stream=True)
    import_keys(d)

    head = out(["git", "-C", d, "rev-parse", "HEAD"]) or ""
    blob = head + _sha_line(HERE / "branding/cobalt.config")
    logo = HERE / "branding/logo.ppm"
    if logo.is_file():
        blob += _sha_line(logo)
    key = hashlib.sha256(blob.encode()).hexdigest()[:16]
    cdir = state.CACHE / "kernel" / key
    om = Path(C.out_dir)
    if (cdir / "vmlinuz").is_file() and (cdir / "modules/version.txt").is_file() and not env("FORCE"):
        print(f"  cached kernel build ({key}): restoring")
        shutil.copy2(cdir / "vmlinuz", om / "vmlinuz")
        cp_a(cdir / "modules", om / "modules")
        C.krel = (om / "modules/version.txt").read_text().strip()
        print(f"  -> Kernel release: {C.krel}")
        return

    print("  downloading + extracting + applying Arch patches (prepare)...", flush=True)
    skip = ["--skippgpcheck"] if env("SKIP_PGP") else []
    plog = state.LOGDIR / "kernel-prepare.log"
    if sh(MP("--nobuild", "--nodeps", "--noconfirm", "--force", *skip), cwd=d, log=plog) != 0:
        print("\n".join(tail(plog, 30)), file=sys.stderr)
        die(f"makepkg prepare failed for linux (log: {plog})")

    src = state.BUILDDIR / "linux" / "src"
    cands = sorted(src.glob(".config")) + sorted(src.glob("*/.config"))
    if not cands:
        die(f"could not locate kernel source tree under {src}")
    tree = cands[0].parent

    print("  merging branding/cobalt.config", flush=True)

    def must(cmd):
        if sh(cmd, cwd=tree, quiet=True) != 0:
            die("kernel configuration failed")

    if logo.is_file():
        print("  using branding/logo.ppm as boot logo")
        shutil.copy2(logo, tree / "drivers/video/logo/logo_linux_clut224.ppm")
        must(["./scripts/config", "--enable", "LOGO", "--enable", "LOGO_LINUX_CLUT224"])
    must(["./scripts/kconfig/merge_config.sh", "-m", ".config", HERE / "branding/cobalt.config"])
    must(["make", "olddefconfig"])
    cfg = set((tree / ".config").read_text().splitlines())
    for opt in ("CONFIG_EFI_STUB=y", "CONFIG_BLK_DEV_INITRD=y", "CONFIG_DEVTMPFS=y"):
        if opt not in cfg:
            print(f"error: {opt} did not survive olddefconfig", file=sys.stderr)
            die("kernel configuration failed")

    krel = (out(["make", "-s", "-C", tree, "kernelrelease"]) or "").strip()
    print(f"  kernel release: {krel}")

    print(f"  building bzImage + modules (-j{C.jobs})... this is the slow part", flush=True)
    blog = state.LOGDIR / "kernel-build.log"
    if sh(["make", "-C", tree, f"-j{C.jobs}", "KBUILD_BUILD_USER=cobalt", "KBUILD_BUILD_HOST=cobalt",
           "bzImage", "modules"], log=blog) != 0:
        print("\n".join(tail(blog, 40)), file=sys.stderr)
        die(f"kernel build failed (full log: {blog})")

    ktmp = state.WORK / "kmod"
    shutil.rmtree(ktmp, ignore_errors=True)
    ktmp.mkdir(parents=True)
    if sh(["make", "-C", tree, f"INSTALL_MOD_PATH={ktmp}", "INSTALL_MOD_STRIP=1", "modules_install"],
          log=blog, append=True) != 0:
        die("modules_install failed")

    mdir = next((p for p in (ktmp / "usr/lib/modules" / krel, ktmp / "lib/modules" / krel) if p.is_dir()), None)
    if mdir is None:
        mdir = next((p for p in ktmp.rglob(krel) if p.is_dir() and "lib/modules" in str(p)), None)
    if mdir is None:
        die(f"modules for {krel} not found under {ktmp}")
    for n in ("build", "source"):
        (mdir / n).unlink(missing_ok=True)

    image = (out(["make", "-s", "-C", tree, "image_name"]) or "").strip()
    shutil.copyfile(tree / image, om / "vmlinuz")
    os.chmod(om / "vmlinuz", 0o644)
    cp_a(mdir, om / "modules")
    (om / "modules/version.txt").write_text(krel + "\n")
    (cdir / "modules").mkdir(parents=True, exist_ok=True)
    shutil.copy2(om / "vmlinuz", cdir / "vmlinuz")
    cp_a(om / "modules", cdir / "modules")
    print(f"  -> Kernel:  {om}/vmlinuz")
    print(f"  -> Modules: {om}/modules (version: {krel})")
    C.krel = krel
