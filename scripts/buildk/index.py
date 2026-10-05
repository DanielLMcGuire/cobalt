from __future__ import annotations

import glob
import re
import subprocess
import time

from . import state
from .util import die, env

INDEX_TTL = 30 * 60

def force_update():
    return env("FORCE_UPDATE") not in ("", "0")

def index_cache_age():
    f = state.IDX / "repo.tsv"
    try:
        if f.stat().st_size == 0:
            return None
        return time.time() - f.stat().st_mtime
    except OSError:
        return None

def sync_dbs_present():
    return bool(glob.glob("/var/lib/pacman/sync/*.db"))

R_BASE, R_VER, R_DEPS, R_LIC, R_PROV = {}, {}, {}, {}, {}

def build_index():
    state.IDX.mkdir(parents=True, exist_ok=True)
    p = subprocess.run(["expac", "-S", "-l", " ", "%n\t%e\t%v\t%D\t%P\t%L"],
                       capture_output=True, text=True, errors="replace")
    if p.returncode != 0:
        die("expac failed: " + p.stderr.strip())
    (state.IDX / "repo.tsv").write_text(p.stdout)
    load_index(p.stdout)

def load_index(text):
    for line in text.splitlines():
        f = line.split("\t")
        f += [""] * (6 - len(f))
        n, b, v, d, p, lic = f[:6]
        if not n:
            continue
        R_BASE[n] = b or n
        R_VER[n] = v
        R_DEPS[n] = d.split()
        R_LIC[n] = lic
        for tok in p.split():
            tok = re.split(r"[<>=]", tok, maxsplit=1)[0]
            R_PROV.setdefault(tok, n)

def tok_of(spec):
    return re.split(r"[<>=]", spec, maxsplit=1)[0]

def repo_resolve(tok):
    tok = tok_of(tok)
    if tok in R_BASE:
        return tok
    return R_PROV.get(tok, "")

def repo_tag(name):
    v = R_VER.get(name, "")
    return v.replace(":", "-") if v else ""
