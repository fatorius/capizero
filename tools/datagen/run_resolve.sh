#!/usr/bin/env bash
# Usage:
#   ./tools/datagen/run_resolve.sh <raw_file> [workers] [out_prefix]
#
# Defaults: workers = all logical cores; out_prefix = raw file name without
# its "_raw.txt" suffix (sp_v1_raw.txt → sp_v1).
#
# Examples:
#   ./tools/datagen/run_resolve.sh sp_v1_raw.txt
#   RESOLVE_DEPTH=10 ./tools/datagen/run_resolve.sh sp_v1_raw.txt 8 sp_v1_d10
#
# On macOS the run is wrapped in `caffeinate -i` so the machine does not
# idle-sleep mid-run (lid-close sleep still applies — keep it open or on
# an external display).

set -euo pipefail

RAW=${1:?usage: $0 <raw_file> [workers] [out_prefix]}
WORKERS=${2:-$( (sysctl -n hw.ncpu 2>/dev/null || nproc) )}
OUT_PREFIX=${3:-$(basename "${RAW}" .txt)}
OUT_PREFIX=${OUT_PREFIX%_raw}

RESOLVE_DEPTH=${RESOLVE_DEPTH:-12}
RESOLVE_TT_MB=${RESOLVE_TT_MB:-16}

# Re-exec under caffeinate once, if available.
if [[ -z "${CAPI_CAFFEINATED:-}" ]] && command -v caffeinate >/dev/null 2>&1; then
    export CAPI_CAFFEINATED=1
    exec caffeinate -i "$0" "$@"
fi

if [[ ! -x ./capi_resolve ]]; then
    echo "error: ./capi_resolve not found — run 'make resolve' first" >&2
    exit 1
fi
if [[ ! -f "${RAW}" ]]; then
    echo "error: ${RAW} not found" >&2
    exit 1
fi

total=$(wc -l < "${RAW}" | tr -d ' ')
final_out="${OUT_PREFIX}_D${RESOLVE_DEPTH}_Resolved.book"
work_dir="${OUT_PREFIX}_resolve_work"
mkdir -p "${work_dir}"

echo "==== Phase B: ${total} positions, ${WORKERS} resolvers at depth ${RESOLVE_DEPTH} ===="
start=$(date +%s)

# Round-robin split, same as run_selfplay.sh (portable to macOS awk).
chunk_prefix="${work_dir}/chunk_"
rm -f "${chunk_prefix}"*
awk -v n="${WORKERS}" -v prefix="${chunk_prefix}" '
    { print > sprintf("%s%02d", prefix, (NR - 1) % n) }
' "${RAW}"

pids=()
for i in $(seq 0 $((WORKERS - 1))); do
    chunk=$(printf "%s%02d" "${chunk_prefix}" "${i}")
    [[ -f "${chunk}" ]] || continue
    ./capi_resolve --in "${chunk}" --out "${chunk}.resolved" \
                   --depth "${RESOLVE_DEPTH}" --tt-mb "${RESOLVE_TT_MB}" \
        > "${chunk}.log" 2>&1 &
    pids+=($!)
done

# Progress line every 5 minutes (output is stdio-buffered, so it lags slightly).
(
    while sleep 300; do
        done_n=$(cat "${chunk_prefix}"*.resolved 2>/dev/null | wc -l | tr -d ' ')
        elapsed=$(( $(date +%s) - start ))
        if (( done_n > 0 )); then
            eta=$(( elapsed * (total - done_n) / done_n ))
            printf "[%s] %d/%d (%d%%) | elapsed %dm | ETA %dh%02dm\n" \
                "$(date '+%F %T')" "${done_n}" "${total}" $(( done_n * 100 / total )) \
                $(( elapsed / 60 )) $(( eta / 3600 )) $(( eta % 3600 / 60 ))
        fi
    done
) &
progress_pid=$!

status=0
for pid in "${pids[@]}"; do
    wait "${pid}" || status=1
done
kill "${progress_pid}" 2>/dev/null || true

end=$(date +%s)
echo "Phase B done in $((end - start))s"

if (( status != 0 )); then
    echo "error: a resolver failed — see ${work_dir}/*.log (chunks kept)" >&2
    exit 1
fi

cat "${chunk_prefix}"*.resolved > "${final_out}"
rm -rf "${work_dir}"

echo "==== Done ===="
echo "Final dataset: ${final_out} ($(wc -l < "${final_out}" | tr -d ' ') positions)"
echo "Feed to tuner: ./capi_tuner ${final_out} --max 0 --passes 1"
