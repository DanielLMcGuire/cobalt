from __future__ import annotations

import json
import time
import urllib.parse
import urllib.request

from .index import tok_of

_aur_cache = {}

def _aur_get(url):
    for attempt in range(3):
        try:
            with urllib.request.urlopen(url, timeout=30) as r:
                return json.load(r)
        except Exception:
            time.sleep(attempt + 1)
    return None

def aur_lookup(tok):
    t = tok_of(tok)
    if t in _aur_cache:
        return _aur_cache[t]
    q = urllib.parse.quote(t, safe="")
    res = None
    for url in (f"https://aur.archlinux.org/rpc/v5/info?arg%5B%5D={q}",
                f"https://aur.archlinux.org/rpc/v5/search/{q}?by=provides"):
        data = _aur_get(url)
        if data and data.get("results"):
            r = data["results"][0]
            res = (r["Name"], r["PackageBase"])
            break
    _aur_cache[t] = res
    return res
