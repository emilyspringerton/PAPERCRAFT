#!/usr/bin/env bash
# build-parena.sh -- real, small, bounded fix for MODDING.md's own honestly-named "Two repos, not
# one" gap: "a modder needs both PAPERCRAFT and PARENA checked out and PARENA's own compiler built
# locally -- there's no vendored/prebuilt parena binary shipped inside PAPERCRAFT itself."
#
# Deliberately NOT a vendored binary (a committed, platform-specific compiler binary is real,
# ongoing maintenance debt this monorepo's own "generated code committed, not binaries" precedent
# doesn't extend to) -- this script automates the exact real Step 1 MODDING.md already documents
# (`cd PARENA && make`) so a modder types one command instead of two, from either repo's own root,
# and gets a real, actionable error instead of a confusing `cd: no such file or directory` if
# PARENA isn't checked out where expected.
#
# Usage:
#   scripts/build-parena.sh              # looks for a sibling ../PARENA checkout (this monorepo's
#                                          # own real layout, /home/fatbaby/{PAPERCRAFT,PARENA})
#   PARENA_DIR=/path/to/PARENA scripts/build-parena.sh   # explicit override
#
# On success, prints the real, resolved absolute path to the built ./parena binary on stdout (and
# nothing else on stdout), so a real caller can do:
#   PARENA_BIN="$(scripts/build-parena.sh)"
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PAPERCRAFT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
PARENA_DIR="${PARENA_DIR:-$PAPERCRAFT_ROOT/../PARENA}"

if [ ! -d "$PARENA_DIR" ]; then
    echo "FATAL: no PARENA checkout found at $PARENA_DIR" >&2
    echo "  Clone it as a sibling of this repo (matching this monorepo's own real layout):" >&2
    echo "    git clone <parena-repo-url> $(dirname "$PARENA_DIR")/PARENA" >&2
    echo "  ...or point PARENA_DIR at an existing checkout:" >&2
    echo "    PARENA_DIR=/path/to/PARENA scripts/build-parena.sh" >&2
    exit 1
fi

if [ ! -f "$PARENA_DIR/Makefile" ]; then
    echo "FATAL: $PARENA_DIR exists but has no Makefile -- is this really a PARENA checkout?" >&2
    exit 1
fi

echo "Building the real PARENA compiler in $PARENA_DIR..." >&2
if ! (cd "$PARENA_DIR" && make build) >&2; then
    echo "FATAL: 'make build' failed in $PARENA_DIR -- see the real compiler error above." >&2
    exit 1
fi

PARENA_BIN="$PARENA_DIR/parena"
if [ ! -x "$PARENA_BIN" ]; then
    echo "FATAL: make build reported success but $PARENA_BIN doesn't exist or isn't executable." >&2
    exit 1
fi

echo "Real PARENA compiler built: $PARENA_BIN" >&2
echo "$PARENA_BIN"
