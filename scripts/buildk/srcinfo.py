from __future__ import annotations

import re
from pathlib import Path

class SrcInfo:
    def __init__(self, path):
        self.base = {}
        self.pkgs = {}
        self.names = []
        self.lines = []
        sec = self.base
        for raw in Path(path).read_text(errors="replace").splitlines():
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            k, sep, v = line.partition(" = ")
            if not sep:
                continue
            if k == "pkgbase":
                sec = self.base
            elif k == "pkgname":
                sec = self.pkgs.setdefault(v, {})
                self.names.append(v)
            sec.setdefault(k, []).append(v)
            self.lines.append((k, v))

    def values(self, pattern):
        rx = re.compile(pattern)
        return [v for k, v in self.lines if rx.fullmatch(k)]

    @property
    def pkgver(self):
        vs = self.values("pkgver")
        return vs[0] if vs else "?"

    def build_deps(self, check=False):
        keys = "depends|makedepends" + ("|checkdepends" if check else "")
        return sorted(set(self.values(rf"(?:{keys})(?:_x86_64)?")))

    def runtime_deps(self, want):
        def dep(sec):
            return sec.get("depends", []) + sec.get("depends_x86_64", [])
        p = dep(self.pkgs.get(want, {}))
        return p if p else dep(self.base)

def is_closed(si, base):
    if base.endswith(("-bin", "-appimage", "-binary")) or "-bin-" in base:
        return "name looks like a prebuilt-binary repack"
    lic = " ".join(si.values("license")) + " "
    if re.search(r"proprietary|commercial|freeware|unfree|eula|nonfree|closed", lic, re.I):
        return f"license: {lic}"
    srcs = si.values(r"source(?:_[a-z0-9_]+)?")
    if any(re.search(r"\.(appimage|deb|rpm|run|exe|msi|snap|dmg)([?#].*)?$", s, re.I) for s in srcs):
        return "sources are prebuilt binaries/installers"
    return None
