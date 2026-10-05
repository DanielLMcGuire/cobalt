from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path

from .state import DEVNULL, Die

def die(msg):
    raise Die(msg)

def warn(msg):
    print(f"  warning: {msg}", file=sys.stderr, flush=True)

def step(msg):
    print(f"\n[*] {msg}", flush=True)

def env(name, default=""):
    return os.environ.get(name, default)

def sh(cmd, *, cwd=None, envv=None, log=None, append=False, quiet=False, quiet_all=False):
    cmd = [str(c) for c in cmd]
    if log:
        with open(log, "ab" if append else "wb") as f:
            return subprocess.run(cmd, cwd=cwd, env=envv, stdout=f, stderr=subprocess.STDOUT).returncode
    so = DEVNULL if (quiet or quiet_all) else None
    se = DEVNULL if quiet_all else None
    return subprocess.run(cmd, cwd=cwd, env=envv, stdout=so, stderr=se).returncode

def out(cmd, *, cwd=None):
    try:
        p = subprocess.run([str(c) for c in cmd], cwd=cwd, capture_output=True,
                           text=True, errors="replace")
    except FileNotFoundError:
        return None
    return p.stdout if p.returncode == 0 else None

def tail(path, n):
    try:
        return Path(path).read_text(errors="replace").splitlines()[-n:]
    except OSError:
        return []

def cp_a(src_dir, dst_dir):
    Path(dst_dir).mkdir(parents=True, exist_ok=True)
    if sh(["cp", "-a", f"{src_dir}/.", f"{dst_dir}/"]) != 0:
        die(f"cp -a {src_dir} -> {dst_dir} failed")

def rm_children(d, keep=()):
    for p in Path(d).iterdir():
        if p.name in keep:
            continue
        if p.is_dir() and not p.is_symlink():
            shutil.rmtree(p, ignore_errors=True)
        else:
            p.unlink(missing_ok=True)

def sudo_clear(d):
    sh(["sudo", "find", d, "-mindepth", "1", "-delete"])

def fmt_elapsed(secs):
    m, s = divmod(int(round(secs)), 60)
    return f"{m} minute{'s' if m != 1 else ''}, {s} second{'s' if s != 1 else ''}"
