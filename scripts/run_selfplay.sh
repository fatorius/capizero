#!/usr/bin/env bash
#
# Two-phase self-play data collector orchestrator.
#
#   Phase A: launches N parallel `capi_selfplay` workers, each playing
#            $GAMES games, writing to its own raw file.
#   Phase B: concatenates Phase A outputs, splits into N chunks, launches
#            N parallel `capi_resolve` workers, concatenates resolved chunks
#            into the final dataset.
#
# The output dataset is named ${OUT_PREFIX}_D${DEPTH}_Resolved.book —
# tuner-compatible, ready for `./capi_tuner <file>`.
#
# Usage:
#   ./scripts/run_selfplay.sh [workers] [games_per_worker] [out_prefix] [extra_selfplay_args...]
#
# Examples:
#   ./scripts/run_selfplay.sh 8 1000 selfplay_run1
#   ./scripts/run_selfplay.sh 8 500 myrun --time-per-move 200 --book openings.epd
#
# Tuning notes:
#   - --time-per-move (default 100ms) controls Phase A search quality vs throughput.
#   - --depth-fixed N (Phase A alternative) for reproducible runs.
#   - Resolver depth (Phase B) is set via RESOLVE_DEPTH below, default 12 to
#     match the E12.33 dataset convention.

set -euo pipefail

WORKERS=${1:-8}
GAMES=${2:-1000}
OUT_PREFIX=${3:-selfplay}
shift 3 2>/dev/null || true
EXTRA_ARGS=("$@")

RESOLVE_DEPTH=${RESOLVE_DEPTH:-12}
RESOLVE_TT_MB=${RESOLVE_TT_MB:-16}

if [[ ! -x ./capi_selfplay ]]; then
    echo "error: ./capi_selfplay not found — run 'make selfplay' first" >&2
    exit 1
fi
if [[ ! -x ./capi_resolve ]]; then
    echo "error: ./capi_resolve not found — run 'make resolve' first" >&2
    exit 1
fi

echo "==== Phase A: ${WORKERS} workers × ${GAMES} games ===="
phase_a_start=$(date +%s)

pids=()
for i in $(seq 1 "${WORKERS}"); do
    seed=$((i * 1000 + RANDOM))
    out_file="${OUT_PREFIX}_raw_${i}.txt"
    ./capi_selfplay --games "${GAMES}" --out "${out_file}" --seed "${seed}" "${EXTRA_ARGS[@]}" \
        > "${OUT_PREFIX}_worker_${i}.log" 2>&1 &
    pids+=($!)
done
for pid in "${pids[@]}"; do
    wait "${pid}"
done

phase_a_end=$(date +%s)
echo "Phase A done in $((phase_a_end - phase_a_start))s"

# Concatenate raw outputs.
raw_combined="${OUT_PREFIX}_raw.txt"
cat "${OUT_PREFIX}"_raw_*.txt > "${raw_combined}"
rm -f "${OUT_PREFIX}"_raw_*.txt
echo "Phase A produced $(wc -l < "${raw_combined}") raw positions in ${raw_combined}"

echo "==== Phase B: ${WORKERS} resolvers at depth ${RESOLVE_DEPTH} ===="
phase_b_start=$(date +%s)

# Split raw file into N roughly-equal chunks. macOS `split` lacks `-n l/N`, so
# we use awk's NR modulo to deal lines round-robin into N output files — same
# net effect (lines distributed evenly, no input duplication).
chunk_prefix="${OUT_PREFIX}_chunk_"
awk -v n="${WORKERS}" -v prefix="${chunk_prefix}" '
    { print > sprintf("%s%02d", prefix, (NR - 1) % n) }
' "${raw_combined}"

pids=()
for i in $(seq 0 $((WORKERS - 1))); do
    chunk=$(printf "%s%02d" "${chunk_prefix}" "${i}")
    [[ -f "${chunk}" ]] || continue
    ./capi_resolve --in "${chunk}" --out "${chunk}.resolved" \
                   --depth "${RESOLVE_DEPTH}" --tt-mb "${RESOLVE_TT_MB}" \
        > "${chunk}.log" 2>&1 &
    pids+=($!)
done
for pid in "${pids[@]}"; do
    wait "${pid}"
done

phase_b_end=$(date +%s)
echo "Phase B done in $((phase_b_end - phase_b_start))s"

final_out="${OUT_PREFIX}_D${RESOLVE_DEPTH}_Resolved.book"
cat "${chunk_prefix}"*.resolved > "${final_out}"
rm -f "${chunk_prefix}"*

echo "==== Done ===="
echo "Final dataset: ${final_out} ($(wc -l < "${final_out}") positions)"
echo "Feed to tuner: ./capi_tuner ${final_out} --max 0 --passes 1"
