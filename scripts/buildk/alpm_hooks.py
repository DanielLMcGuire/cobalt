from __future__ import annotations

import fnmatch
import os
import re
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

from . import state
from .index import tok_of
from .install import PKG_INFO
from .state import DEVNULL
from .util import out, sh, warn

def parse_hook(path):
    h = {"triggers": [], "When": [], "Exec": [], "Depends": [], "NeedsTargets": False}
    sec, cur = None, None
    for raw in Path(path).read_text(errors="replace").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        if line == "[Trigger]":
            sec, cur = "t", {"Operation": [], "Type": "", "Target": []}
            h["triggers"].append(cur)
            continue
        if line == "[Action]":
            sec = "a"
            continue
        k, sep, v = line.partition("=")
        k, v = k.strip(), v.strip()
        if sec == "t" and sep:
            if k in ("Operation", "Target"):
                cur[k].append(v)
            elif k == "Type":
                cur["Type"] = v
        elif sec == "a":
            if k == "NeedsTargets":
                h["NeedsTargets"] = True
            elif sep and k in ("When", "Exec", "Depends"):
                h[k].append(v)
    return h

def hook_matches(h, names, paths):
    hit = set()
    for t in h["triggers"]:
        if "Install" not in t["Operation"]:
            continue
        pool = names if t["Type"] == "Package" else paths
        for pat in t["Target"]:
            rx = re.compile(fnmatch.translate(pat))
            hit.update(x for x in pool if rx.match(x))
    return sorted(hit)

_CHROOT_ENV = ["PATH=/usr/local/sbin:/usr/local/bin:/usr/bin", "HOME=/root", "TMPDIR=/tmp", "LC_ALL=C"]

def _chroot(argv, input_text=None):
    chroot_bin = shutil.which("chroot", path=os.environ.get("PATH", "") + ":/usr/sbin:/sbin") or "chroot"
    cmd = ["sudo", "env", "-i", *_CHROOT_ENV, chroot_bin, state.TARGET_OVERLAY, *[str(a) for a in argv]]
    kw = {"input": input_text} if input_text is not None else {"stdin": DEVNULL}
    p = subprocess.run(cmd, capture_output=True, text=True, errors="replace", **kw)
    return p.returncode, (p.stdout or "") + (p.stderr or "")

def _first_line(text, limit=160):
    for l in text.splitlines():
        if l.strip():
            return l.strip()[:limit]
    return "no output"

def run_install_scripts():
    ov = Path(state.TARGET_OVERLAY)
    log = state.LOGDIR / "postinstall.log"
    names = list(PKG_INFO)
    name_set = set(names)
    lines = []
    failed = []
    n_s = n_h = 0

    if not (ov / "usr/bin/bash").exists():
        warn("no /usr/bin/bash in the overlay: install scripts and hooks were not run")
        return [n for n in names if PKG_INFO[n]["install"]]

    devnull = ov / "dev" / "null"
    stub = False
    if not (devnull.exists() or devnull.is_symlink()):
        (ov / "dev").mkdir(parents=True, exist_ok=True)
        stub = sh(["sudo", "install", "-m", "666", "/dev/null", devnull], quiet_all=True) == 0
    try:
        rc, o = _chroot(["/usr/bin/bash", "-c", ":"])
        if rc != 0:
            warn("cannot chroot into the overlay (needs CAP_SYS_CHROOT): install scripts and hooks "
                 f"were not run ({_first_line(o)})")
            return [n for n in names if PKG_INFO[n]["install"]]

        for name in names:
            info = PKG_INFO[name]
            if not info["install"]:
                continue
            script = out(["bsdtar", "-xOf", info["file"], ".INSTALL"]) or ""
            if not re.search(r"\bpost_install\b", script):
                continue
            tmpf = ov / "tmp" / f".buildk-{name}.install"
            tmpf.parent.mkdir(parents=True, exist_ok=True)
            tmpf.write_text(script)
            rc, o = _chroot(["/usr/bin/bash", "-c",
                             'source "$1"; if declare -F post_install >/dev/null; then post_install "$2"; fi',
                             "bash", f"/tmp/{tmpf.name}", info["ver"]])
            tmpf.unlink(missing_ok=True)
            n_s += 1
            lines.append(f"== scriptlet {name} post_install {info['ver']}: rc={rc}\n{o}")
            if rc != 0:
                failed.append(f"{name} (post_install: {_first_line(o)})")

        hooks = {}
        for d in (ov / "usr/share/libalpm/hooks", ov / "etc/pacman.d/hooks"):
            if d.is_dir():
                for f in d.glob("*.hook"):
                    hooks[f.name] = f
        paths = [x for i in PKG_INFO.values() for x in i["paths"]]
        for fname in sorted(hooks):
            try:
                h = parse_hook(hooks[fname])
            except OSError:
                continue
            if "PostTransaction" not in h["When"] or not h["Exec"]:
                continue
            targets = hook_matches(h, names, paths)
            if not targets:
                continue
            missing_deps = [d for d in h["Depends"] if tok_of(d) not in name_set]
            if missing_deps:
                lines.append(f"== hook {fname}: skipped, needs {' '.join(missing_deps)}")
                continue
            try:
                argv = shlex.split(h["Exec"][0])
            except ValueError:
                continue
            if not argv or (argv[0].startswith("/") and not os.path.lexists(str(ov) + argv[0])):
                lines.append(f"== hook {fname}: skipped, {argv[0] if argv else '?'} is not in the overlay")
                continue
            rc, o = _chroot(argv, input_text=("\n".join(targets) + "\n") if h["NeedsTargets"] else None)
            n_h += 1
            lines.append(f"== hook {fname} ({len(targets)} trigger target(s)): rc={rc}\n{o}")
            if rc != 0:
                failed.append(f"hook {fname}: {_first_line(o)}")
    finally:
        if stub:
            sh(["sudo", "rm", "-f", devnull], quiet_all=True)
        log.write_text("\n".join(lines) + "\n")

    print(f"  install scripts: ran {n_s} scriptlet(s) and {n_h} hook(s) in a chroot of the overlay; log: {log}")
    for f in failed:
        print(f"    failed: {f}", file=sys.stderr)
    return failed
