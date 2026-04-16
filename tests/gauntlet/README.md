# Gauntlet testing for capizero

Gauntlet = capizero plays a match against each of several external
opponents. Unlike SPRT (which measures *relative* Elo between two versions),
a gauntlet gives an *absolute-ish* Elo anchor — you can say "capizero is
around 1800" instead of "capizero gained 30 Elo over the previous build".

The default configuration uses the bundled `./stockfish` binary at several
reduced `UCI_Elo` settings as opponents. This is a pragmatic choice:
Stockfish is already in the repo, and its `UCI_LimitStrength` + `UCI_Elo`
options let you pick the rating target explicitly. See [Caveats](#caveats)
below for why the number you get is approximate, not a published rating.

## Prerequisites

- **Fastchess** — same as SPRT; see [`../sprt/README.md` → Installing
  Fastchess](../sprt/README.md#installing-fastchess).
- **`./stockfish`** at the repo root (already present).
- **Built capizero binary** at the repo root
  (`make build` → `capizero_<version>`).
- **Opening book** — reuses `tests/sprt/book/`. UHO preferred if present,
  else falls back to `starter.epd`.

## Quick start

```bash
# Full gauntlet (default: 100 rounds × 2 games × 5 opponents = 1000 games).
tests/gauntlet/run_gauntlet.sh

# Fast smoke test (10 games per opponent = 50 games total).
ROUNDS=5 tests/gauntlet/run_gauntlet.sh
```

Arguments and env vars: see the header of
[`run_gauntlet.sh`](./run_gauntlet.sh).

## Configuring opponents

Opponents live in [`opponents.conf`](./opponents.conf). It's sourced as
bash — each entry is a complete `fastchess -engine ...` argument sequence.
Default is Stockfish at `UCI_Elo` ∈ {1320, 1500, 1700, 1900, 2100}.

Add or remove opponents by editing the `OPPONENTS` array. You can plug in
any UCI engine:

```bash
OPPONENTS=(
    -engine cmd=./stockfish         name=sf1500 proto=uci option.UCI_LimitStrength=true option.UCI_Elo=1500
    -engine cmd=/path/to/vice       name=vice   proto=uci
    -engine cmd=/path/to/rustic     name=rustic proto=uci
)
```

## Interpreting the output

Fastchess prints incremental updates and, at the end, a cross-table with
per-engine Elo estimates. Example final output:

```
   Engine        Games    Win   Loss  Draw    Score   Elo
1  capizero      1000     580   320   100     63.0%   92
2  sf2100        200      150   40    10      77.5%  210
3  sf1900        200      120   60    20      65.0%  108
4  sf1700        200      90    90    20      50.0%    0
5  sf1500        200      50    130   20      30.0% -147
6  sf1320        200      20    170   10      12.5% -336
```

Reading this:

- **capizero's row** gives an overall Elo relative to the field's average.
- **Per-opponent score percentage** reveals where capizero sits: it scored
  50% vs sf1700, so it's ~equal to Stockfish-at-1700. That's capizero's
  anchored absolute rating (≈1700 ± noise).
- **Lopsided scores** (e.g. 12.5% vs sf1320) tell you the anchor is too
  weak to be informative. Adjust `opponents.conf` to include stronger
  opponents if you're saturating at the top.
- **50% ± 5%** is the target band for informative opponents — that's
  where the Elo estimate has the tightest signal.

## Caveats

### Stockfish's `UCI_Elo` is approximate

Stockfish's strength-limiter is implemented by randomizing move selection
and reducing search depth, *calibrated against human databases*. The
number you set is a target, not a measured rating. The real Elo at
UCI_Elo=1700 on your hardware, at your time control, against your engine
is probably within ±150 Elo of 1700 — fine for ballpark anchoring, not
useful for sub-50-Elo discrimination.

### Time control matters

A given UCI_Elo means something different at 1+0.1 vs 60+0.6. At very
short TC Stockfish's limiter is noisier; at very long TC it approaches
its uncapped strength more than you'd expect. `10+0.1` is a reasonable
default for quick iteration.

### Use SPRT for patches, gauntlet for snapshots

Gauntlet is the wrong tool for comparing version N+1 vs N — the noise
floor is too high and the absolute anchor moves every time Stockfish is
updated. Use [`../sprt/run_sprt.sh`](../sprt/run_sprt.sh) for
patch-level comparisons. Gauntlet answers the different question *where
are we now*, run occasionally to track absolute progress.

### Only one instance of Stockfish

The default setup runs the same `./stockfish` binary with different
options. Fastchess spawns a fresh process per game per engine slot, so
there's no shared-state issue. But if Stockfish is updated (or you
re-download it), past gauntlet results are no longer directly comparable.

## Output

PGN files accumulate in `tests/gauntlet/out/gauntlet_<timestamp>.pgn`.
Gitignored except for the `.gitignore` stub; don't commit them.
