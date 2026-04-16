#!/usr/bin/env bash
#
# SPRT (Sequential Probability Ratio Test) self-test for capizero.
# Drives Fastchess to play games between a baseline binary and a candidate
# binary, accepting/rejecting the null hypothesis that the candidate is
# within [elo0, elo1] Elo of the baseline.
#
# Usage:
#   tests/sprt/run_sprt.sh <baseline_binary> <candidate_binary> \
#                          [tc]          [elo0] [elo1] [concurrency]
#
# Defaults:
#   tc           10+0.1            (10 sec + 0.1 sec increment)
#   elo0         0                 (null hypothesis lower bound)
#   elo1         5                 (null hypothesis upper bound: gain 5 Elo)
#   concurrency  (nproc) / 2       (leave some cores for the OS)
#
# Environment overrides:
#   FASTCHESS    path to fastchess binary (default: fastchess on PATH)
#   BOOK         path to opening book (default: UHO if present, else starter)
#   HASH         TT hash MB per engine (default: 64)
#   ALPHA BETA   SPRT type-I / type-II error rates (default: 0.05 / 0.05)
#   ROUNDS       max rounds cap (default: 40000)
#
# Example: compare HEAD against a previous build
#   git stash                      # if you have uncommitted changes
#   git checkout main
#   make build && cp capizero_* /tmp/baseline
#   git checkout <your-branch>
#   make build && cp capizero_* /tmp/candidate
#   git stash pop
#   tests/sprt/run_sprt.sh /tmp/baseline /tmp/candidate
#
# Exit code: 0 = H1 accepted (candidate gains elo), 1 = H0 accepted (no
# gain), 2 = inconclusive, anything else = script error.

set -euo pipefail

BASELINE="${1:-}"
CANDIDATE="${2:-}"
TC="${3:-10+0.1}"
ELO0="${4:-0}"
ELO1="${5:-5}"
CONCURRENCY="${6:-}"

if [ -z "$BASELINE" ] || [ -z "$CANDIDATE" ]; then
    cat >&2 <<USAGE
Usage: $0 <baseline_binary> <candidate_binary> [tc] [elo0] [elo1] [concurrency]

Defaults: tc=10+0.1 elo0=0 elo1=5 concurrency=(nproc)/2
Environment: FASTCHESS BOOK HASH ALPHA BETA ROUNDS
USAGE
    exit 64
fi

if [ ! -x "$BASELINE" ]; then
    echo "error: baseline binary not executable: $BASELINE" >&2
    exit 1
fi
if [ ! -x "$CANDIDATE" ]; then
    echo "error: candidate binary not executable: $CANDIDATE" >&2
    exit 1
fi

REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BOOK_DIR="$REPO_ROOT/tests/sprt/book"
OUT_DIR="$REPO_ROOT/tests/sprt/out"
mkdir -p "$OUT_DIR"

FASTCHESS="${FASTCHESS:-fastchess}"
if ! command -v "$FASTCHESS" >/dev/null 2>&1; then
    cat >&2 <<EOF
error: fastchess not found on PATH (or in FASTCHESS env var)

quick install (from source):
  git clone https://github.com/Disservin/fastchess.git
  cd fastchess && make -j && sudo cp fastchess /usr/local/bin/

full install guide (macOS / Linux / package managers / verification):
  tests/sprt/README.md  (section: "Installing Fastchess")
EOF
    exit 1
fi

if [ -z "${BOOK:-}" ]; then
    if [ -f "$BOOK_DIR/UHO_Lichess_4852_v1.epd" ]; then
        BOOK="$BOOK_DIR/UHO_Lichess_4852_v1.epd"
    else
        BOOK="$BOOK_DIR/starter.epd"
    fi
fi
if [ ! -f "$BOOK" ]; then
    echo "error: opening book not found: $BOOK" >&2
    echo "run: bash tests/sprt/book/download_uho.sh (or use the committed starter.epd)" >&2
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
ALPHA="${ALPHA:-0.05}"
BETA="${BETA:-0.05}"
ROUNDS="${ROUNDS:-40000}"

STAMP=$(date +%Y%m%d_%H%M%S)
PGN_OUT="$OUT_DIR/sprt_${STAMP}.pgn"

echo "-- SPRT configuration"
echo "   baseline    : $BASELINE"
echo "   candidate   : $CANDIDATE"
echo "   time control: $TC"
echo "   elo bounds  : [$ELO0, $ELO1]  (alpha=$ALPHA beta=$BETA)"
echo "   concurrency : $CONCURRENCY"
echo "   hash / side : ${HASH}MB"
echo "   book        : $BOOK"
echo "   rounds cap  : $ROUNDS"
echo "   pgn output  : $PGN_OUT"
echo

"$FASTCHESS" \
    -engine cmd="$CANDIDATE" args="uci" name=candidate proto=uci \
    -engine cmd="$BASELINE"  args="uci" name=baseline  proto=uci \
    -each tc="$TC" option.Hash="$HASH" \
    -rounds "$ROUNDS" -games 2 -repeat \
    -openings file="$BOOK" format=epd order=random \
    -sprt elo0="$ELO0" elo1="$ELO1" alpha="$ALPHA" beta="$BETA" \
    -concurrency "$CONCURRENCY" \
    -pgnout file="$PGN_OUT"
