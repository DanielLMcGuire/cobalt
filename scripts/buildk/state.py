from __future__ import annotations

import subprocess
from pathlib import Path
from types import SimpleNamespace

HERE = Path(__file__).resolve().parents[2]

IMG = "cobalt-buildk"

DEVNULL = subprocess.DEVNULL

C = SimpleNamespace(
    kver="latest", out_dir="./kernel_out", no_xxc=False,
    pkg_args=[], local_pkgs=[], orig_args=[],
    engine="", mount_args=[], jobs=1, krel="",
)

class Die(Exception):
    pass

class CloneError(Exception):
    pass

class TagMissing(Exception):
    pass

SEEN_TOK = set()

SEEN_NAME = set()

WANT = {}

BASE_SRC, BASE_DIR, SI = {}, {}, {}

SKIPPED = {}

LOCAL_NAMES = set()

BUILT, BUILDING = set(), set()

BASES = []

MISSING = []

FAILED = []

CACHE = WORK = PKGDEST = SRCDEST = BUILDDIR = LOGDIR = IDX = None

MAKEPKG_CONF = TARGET_OVERLAY = TMP = None

INSTALL_MODE = "pacstrap"
