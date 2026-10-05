from __future__ import annotations

import os
import sys
import threading

from . import state
from .makepkg import refresh_sums
from .state import HERE
from .util import die, sh, tail, warn

BASE_NOTES = {}

_notes_lock = threading.Lock()

def add_note(base, kind, msg):
    with _notes_lock:
        BASE_NOTES.setdefault(base, []).append((kind, msg))

def flush_notes(base):
    with _notes_lock:
        notes = BASE_NOTES.pop(base, [])
    for kind, msg in notes:
        if kind == "warn":
            warn(msg)
        else:
            print(f"  {msg}", flush=True)

def run_hook(base, d, stream):
    hook = HERE / "pkghooks" / f"{base}.sh"
    if not hook.is_file():
        return
    add_note(base, "info", f"hook: pkghooks/{base}.sh")

    interpreter = ["bash"]
    try:
        with open(hook, "r", encoding="utf-8", errors="ignore") as f:
            first_line = f.readline().strip()
            if first_line.startswith("#!"):
                parsed_shebang = first_line[2:].strip().split()
                if parsed_shebang:
                    interpreter = parsed_shebang
    except Exception:
        pass

    cmd = interpreter + [hook]

    henv = {**os.environ, "PKGBASE": base}
    if stream:
        flush_notes(base)
        rc = sh(cmd, cwd=d, envv=henv)
    else:
        hlog = state.LOGDIR / f"hook-{base}.log"
        rc = sh(cmd, cwd=d, envv=henv, log=hlog)
        if rc != 0:
            for l in tail(hlog, 20):
                print("    " + l, file=sys.stderr)
    if rc != 0:
        die(f"hook pkghooks/{base}.sh failed")

    marker = d / ".buildk-refresh-sums"
    if marker.exists():
        marker.unlink()
        if not refresh_sums(d):
            die(f"{base}: could not regenerate checksums after hook")
