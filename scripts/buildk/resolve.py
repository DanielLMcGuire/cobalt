from __future__ import annotations

import re
import threading
from concurrent.futures import ThreadPoolExecutor

from .aur import aur_lookup
from .index import R_BASE, R_DEPS, repo_resolve, repo_tag, tok_of
from .pkghooks import add_note, flush_notes, run_hook
from .recipes import clone_pkg
from .srcinfo import SrcInfo, is_closed
from .state import BASES, BASE_DIR, BASE_SRC, C, CloneError, LOCAL_NAMES, MISSING, SEEN_NAME, SEEN_TOK, SI, SKIPPED, TagMissing, WANT
from .util import die, env, out, warn

_base_locks = {}

_base_locks_guard = threading.Lock()

def _base_lock(base):
    with _base_locks_guard:
        return _base_locks.setdefault(base, threading.Lock())

def prep_base(src, base, pin_ref="", stream=True):
    with _base_lock(base):
        if base in BASE_DIR:
            return
        try:
            d = clone_pkg(src, base, pin_ref)
        except TagMissing:
            add_note(base, "warn", f"{base}: tag '{pin_ref}' not found, using latest recipe")
            try:
                d = clone_pkg(src, base)
            except CloneError as e:
                die(f"cannot clone {src} recipe for {base}\n  {e}")
        except CloneError as e:
            die(f"cannot clone {src} recipe for {base}\n  {e}")
        run_hook(base, d, stream)
        o = out(["makepkg", "--printsrcinfo"], cwd=d)
        if not o:
            die(f"{base}: cannot generate .SRCINFO (broken PKGBUILD?)")
        (d / ".SRCINFO").write_text(o)
        SI[base] = SrcInfo(d / ".SRCINFO")
        BASE_SRC[base] = src
        BASE_DIR[base] = d

def resolve_one(job):
    spec, by = job
    force = ""
    if spec.startswith("aur:"):
        force, spec = "aur", spec[4:]
    elif spec.startswith("arch:"):
        force, spec = "arch", spec[5:]
    tok = tok_of(spec)
    name = base = src = ""
    if force != "aur":
        n = repo_resolve(tok)
        if n:
            src, name, base = "arch", n, R_BASE[n]
    if not src and force != "arch":
        got = aur_lookup(tok)
        if got:
            src, (name, base) = "aur", got
    r = {"tok": tok, "by": by, "src": src, "name": name, "base": base,
         "missing": not src, "dup": False, "closed": None, "rt": []}
    if not src or name in SEEN_NAME:
        r["dup"] = bool(src)
        return r
    pin = repo_tag(name) if (src == "arch" and env("PIN", "1") != "0") else ""
    prep_base(src, base, pin, stream=False)
    r["closed"] = is_closed(SI[base], base)
    if src == "arch":
        r["rt"] = list(R_DEPS.get(name, []))
    else:
        r["rt"] = SI[base].runtime_deps(name)
    return r

def process_queue(initial):
    n = int(env("RESOLVE_JOBS") or C.jobs)
    wave = list(initial)
    with ThreadPoolExecutor(max_workers=max(1, n)) as pool:
        while wave:
            jobs = []
            for spec, by in wave:
                tok = tok_of(re.sub(r"^(aur|arch):", "", spec))
                if tok in SEEN_TOK or tok in SEEN_NAME or tok in LOCAL_NAMES:
                    continue
                SEEN_TOK.add(tok)
                jobs.append((spec, by))
            if not jobs:
                break
            print(f"  resolving {len(jobs)} package(s) with {min(n, len(jobs))} worker(s)", flush=True)
            results = list(pool.map(resolve_one, jobs))

            wave = []
            for r in results:
                if r["base"]:
                    flush_notes(r["base"])
                if r["missing"]:
                    warn(f"package '{r['tok']}' not found in Arch repos or AUR (needed by {r['by']})")
                    MISSING.append(r["tok"])
                    continue
                name, base, src, by = r["name"], r["base"], r["src"], r["by"]
                if name in SEEN_NAME:
                    continue
                SEEN_NAME.add(name)
                if r["closed"] and not env("ALLOW_BINARY"):
                    SKIPPED[name] = f"{r['closed']} (needed by {by})"
                    print(f"  skip {name} [{src}]: {r['closed']}")
                    continue
                if base not in WANT:
                    BASES.append(base)
                WANT.setdefault(base, []).append(name)
                print(f"  + {name} [{src}{', needed by ' + by if by else ''}]")
                wave.extend((d, name) for d in r["rt"])
