# SPRT testing for capizero

SPRT (Sequential Probability Ratio Test) is the standard way competitive
chess engines decide whether a patch gains or loses Elo. Instead of playing
a fixed number of games and computing mean Elo, SPRT plays games
incrementally and stops as soon as it can statistically confirm H1
(candidate is stronger) or H0 (candidate is within the null-hypothesis
range). This is fast on obvious wins/losses and gives bounded error rates.

## Prerequisites

- **Fastchess** — the SPRT driver. See [Installing Fastchess](#installing-fastchess) below.
- **A baseline binary and a candidate binary**. The candidate is the build
  containing the change you want to test; the baseline is a prior build
  (usually from `main` or a tagged release).
- **An opening book** (`book/starter.epd` is committed; for serious runs
  download UHO via `book/download_uho.sh` — see [book/README.md](book/README.md)).

## Installing Fastchess

Fastchess (https://github.com/Disservin/fastchess) is a chess tournament
manager — roughly the successor to cutechess-cli, but faster and with
native SPRT support. `run_sprt.sh` expects a `fastchess` binary on `PATH`
(or pointed at via the `FASTCHESS` env var).

First check whether it's already installed:

```
fastchess --version
```

If the command works, you're done — skip to [Typical workflow](#typical-workflow).

### Building from source (canonical, works everywhere)

Fastchess is not yet packaged in most distributions, so source build is the
expected path. Requires a C++17 compiler (g++ ≥ 9, clang ≥ 10) and `make`.

```bash
# Install build deps (pick your platform):
#   macOS  : xcode-select --install           # provides clang + make
#   Ubuntu : sudo apt install build-essential git
#   Fedora : sudo dnf install gcc-c++ make git
#   Arch   : sudo pacman -S base-devel git

git clone https://github.com/Disservin/fastchess.git
cd fastchess
make -j

# Install system-wide (optional):
sudo cp fastchess /usr/local/bin/

# ... or keep it in the clone and point FASTCHESS at it:
export FASTCHESS=$PWD/fastchess
```

Build takes roughly 1–2 minutes on a modern machine.

### Package managers

At time of writing Fastchess isn't in mainstream repositories, but this
changes; check yours before compiling:

- **Homebrew** (macOS): `brew search fastchess` — not in core as of early 2026,
  but a third-party tap may exist.
- **AUR** (Arch): search for `fastchess-git`.
- **MSYS2** (Windows): not currently packaged; use WSL or MinGW source build.

### Verifying

```
fastchess --version
fastchess --help | head -20
```

If the `fastchess` command is installed somewhere not on `PATH`, point the
SPRT script at it:

```
FASTCHESS=/path/to/fastchess tests/sprt/run_sprt.sh ...
```

## Typical workflow

```bash
# 1. Build the baseline (prior build / main).
git checkout main
make build
cp capizero_* /tmp/baseline
chmod +x /tmp/baseline

# 2. Build the candidate (your branch with the patch).
git checkout my-patch
make build
cp capizero_* /tmp/candidate
chmod +x /tmp/candidate

# 3. Run SPRT. Defaults: tc=10+0.1, elo0=0, elo1=5, concurrency=nproc/2.
tests/sprt/run_sprt.sh /tmp/baseline /tmp/candidate
```

Fastchess streams per-game updates including the current LLR (log-likelihood
ratio). Typical thresholds at α=β=0.05: LLR ≥ +2.94 → accept H1 (candidate
gains), LLR ≤ −2.94 → accept H0 (no gain).

## Exit codes

| code | meaning |
|------|---------|
| 0    | H1 accepted — candidate is >= `elo1` stronger than baseline |
| 1    | H0 accepted — candidate is <= `elo0` stronger (no gain) |
| 2    | inconclusive — reached round cap without deciding |
| 64   | script usage error (missing args) |
| other| setup error (missing binary, book, fastchess, etc.) |

## Interpreting Fastchess output

Fastchess prints a status line like:

```
LLR:  0.46 (-2.94, 2.94) [0.00, 5.00]
ELO:  4.23 +-  8.12 (95%)
Games: 120  W: 45  L: 40  D: 35  Penta: [3, 12, 26, 15, 4]
```

- **LLR** — log-likelihood ratio. Test stops when it crosses ±2.94 (given
  α=β=0.05).
- **ELO ± CI** — point estimate with 95% confidence interval. Widens with
  few games, narrows as data accumulates.
- **Penta** — pentanomial counts (LL, LD, DD/WL, WD, WW from pairs of games
  played from the same opening with reversed colors). Fastchess uses these
  for SPRT rather than raw W/L/D because they reduce variance.

## Sanity test: "null patch"

To confirm the harness is working, run SPRT with the same binary on both
sides and very tight bounds:

```bash
make build
cp capizero_* /tmp/same
tests/sprt/run_sprt.sh /tmp/same /tmp/same 10+0.1 -5 5 4
```

This should converge to "H0 accepted" (or inconclusive) with an Elo point
estimate close to 0. If it consistently shows a non-zero Elo or decisive
result, something is wrong with the harness (book imbalance, determinism
bug, etc).

## Time controls

`10+0.1` (10-second base, 0.1-second increment) is a common default for
fast iteration. Typical ranges used in engine development:

| tc       | purpose                                       |
|----------|-----------------------------------------------|
| `5+0.05` | very fast smoke tests — high noise            |
| `10+0.1` | default for SPRT — reasonable balance         |
| `30+0.3` | cleaner signal, needs more wall-clock time    |
| `60+0.6` | "LTC" — used for confirmation runs            |

Longer time controls reduce per-game noise but cost more wall-clock.

## Reproducibility

Capizero does not seed `rand()` (see comment in [src/hash.cpp:24](../../src/hash.cpp#L24)),
so Zobrist keys — and therefore node counts from bench — are deterministic
across runs. Two builds from the same source code will produce identical
`./capizero bench` output. This is intentional for SPRT: it lets you
confirm that a candidate differs from the baseline only in the ways you
expect.

## Known limitations

- UCI `stop` is a no-op in capizero (engine is single-threaded and does not
  poll stdin mid-search). Fastchess does not rely on `stop` when games use
  `tc`/`movetime`, so this is fine for SPRT.
- Capizero always promotes to queen, even if the opponent-requested move is
  underpromotion. At engine-vs-engine play this rarely comes up.

## Output

PGN files are written to `tests/sprt/out/sprt_<timestamp>.pgn`. The
directory is gitignored in spirit — the committed folder structure exists
so the script has somewhere to write — but PGNs should not be committed.
