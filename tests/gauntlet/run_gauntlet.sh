#!/usr/bin/env bash
#
# Gauntlet: capizero plays a match against each opponent listed in
# opponents.conf (Stockfish at reduced UCI_Elo by default). Output is a
# cross-table and Fastchess's Elo estimate for each participant, giving
# an absolute-ish rating anchor for capizero.
#
# Usage:
#   tests/gauntlet/run_gauntlet.sh [candidate_binary] [tc] [rounds] [concurrency]
#
# Defaults:
#   candidate_binary   ./capizero_<version>   (auto-detected at repo root)
#   tc                 10+0.1
#   rounds             100       (each round = 2 games, reversed colors)
#   concurrency        (nproc) / 2
#
# Environment overrides:
#   FASTCHESS          fastchess binary (default: fastchess on PATH)
#   BOOK               opening book path (default: UHO if present, else starter)
#   CONF               opponents config (default: tests/gauntlet/opponents.conf)
#   HASH               TT hash MB per engine (default: 64)
#
# Quick smoke test (10 games per pair):
#   ROUNDS=5 tests/gauntlet/run_gauntlet.sh
#
# See README.md for how to interpret the output and caveats around the
# UCI_Elo anchor.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$REPO_ROOT"

CANDIDATE="${1:-}"
TC="${2:-10+0.1}"
ROUNDS="${3:-${ROUNDS:-100}}"
CONCURRENCY="${4:-}"

if [ -z "$CANDIDATE" ]; then
    # Auto-detect the most recent capizero_* binary at the repo root.
    CANDIDATE=$(ls -1t capizero_* 2>/dev/null | head -n1 || true)
    if [ -z "$CANDIDATE" ]; then
        echo "error: no capizero_* binary in repo root" >&2
        echo "build one first: make build  (or pass the path as arg 1)" >&2
        exit 1
    fi
    CANDIDATE="$REPO_ROOT/$CANDIDATE"
fi

if [ ! -x "$CANDIDATE" ]; then
    echo "error: candidate binary not executable: $CANDIDATE" >&2
    exit 1
fi

FASTCHESS="${FASTCHESS:-fastchess}"
if ! command -v "$FASTCHESS" >/dev/null 2>&1; then
    cat >&2 <<EOF
error: fastchess not found on PATH (or in FASTCHESS env var)

install guide: tests/sprt/README.md  (section: "Installing Fastchess")
EOF
    exit 1
fi

CONF="${CONF:-$REPO_ROOT/tests/gauntlet/opponents.conf}"
if [ ! -f "$CONF" ]; then
    echo "error: opponents config not found: $CONF" >&2
    exit 1
fi

OPPONENTS=()
# shellcheck source=/dev/null
source "$CONF"
if [ "${#OPPONENTS[@]}" -eq 0 ]; then
    echo "error: $CONF did not populate the OPPONENTS array" >&2
    exit 1
fi

BOOK_DIR="$REPO_ROOT/tests/sprt/book"
if [ -z "${BOOK:-}" ]; then
    if [ -f "$BOOK_DIR/UHO_Lichess_4852_v1.epd" ]; then
        BOOK="$BOOK_DIR/UHO_Lichess_4852_v1.epd"
    else
        BOOK="$BOOK_DIR/starter.epd"
    fi
fi
if [ ! -f "$BOOK" ]; then
    echo "error: opening book not found: $BOOK" >&2
    echo "run: bash tests/sprt/book/download_uho.sh (or use starter.epd)" >&2
    exit 1
fi

if [ -z "$CONCURRENCY" ]; then
    if command -v nproc >/dev/null 2>&1; then
        CONCURRENCY=$(( $(nproc) / 2 ))
    elif command -v sysctl >/dev/null 2>&1; then
        CONCURRENCY=$(( $(sysctl -n hw.ncpu) / 2 ))
    else
        CONCURRENCY=2
    fi
    [ "$CONCURRENCY" -lt 1 ] && CONCURRENCY=1
fi

HASH="${HASH:-64}"
STAMP=$(date +%Y%m%d_%H%M%S)
OUT_DIR="$REPO_ROOT/tests/gauntlet/out"
mkdir -p "$OUT_DIR"
PGN_OUT="$OUT_DIR/gauntlet_${STAMP}.pgn"

echo "-- Gauntlet configuration"
echo "   candidate   : $CANDIDATE"
echo "   opponents   : $(grep -cE '^\s*-engine' "$CONF") entries"
echo "   time control: $TC"
echo "   rounds      : $ROUNDS  (x 2 games per pairing, reversed colors)"
echo "   concurrency : $CONCURRENCY"
echo "   hash / side : ${HASH}MB"
echo "   book        : $BOOK"
echo "   pgn output  : $PGN_OUT"
echo

"$FASTCHESS" \
    -engine cmd="$CANDIDATE" args="uci" name=capizero proto=uci \
    "${OPPONENTS[@]}" \
    -each tc="$TC" option.Hash="$HASH" \
    -rounds "$ROUNDS" -games 2 -repeat \
    -openings file="$BOOK" format=epd order=random \
    -tournament gauntlet \
    -ratinginterval 10 \
    -concurrency "$CONCURRENCY" \
    -pgnout file="$PGN_OUT"
