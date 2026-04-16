# Opening books for SPRT testing

Two books are available:

## `starter.epd` (committed, ~40 positions)

A small curated set of common opening positions (2–5 plies deep) covering
major opening families. Good enough for quick sanity runs and smoke tests,
but the position count is low — with many SPRT rounds the engine will play
from the same positions repeatedly, reducing diversity.

Regenerate or extend by editing the `OPENINGS` heredoc in
[`generate_starter.sh`](./generate_starter.sh) and re-running:

```
bash tests/sprt/book/generate_starter.sh
```

The generator uses the bundled `./stockfish` binary (at repo root) to convert
UCI move sequences into correct FENs.

## `UHO_Lichess_4852_v1.epd` (download, ~4852 positions)

The UHO (Unbalanced Human Openings) suite is a public test book widely used
in engine SPRT work (OpenBench, CCRL, etc.). Prefer it over `starter.epd`
for any run longer than a quick smoke test.

Fetch it with:

```
bash tests/sprt/book/download_uho.sh
```

The script pulls from the Stockfish books repository. If the URL breaks,
check https://github.com/official-stockfish/books for an up-to-date mirror
and edit `download_uho.sh`.

## Which book does `run_sprt.sh` use?

`run_sprt.sh` prefers `UHO_Lichess_4852_v1.epd` if present and falls back to
`starter.epd`. Override with `BOOK=<path>` in the environment.
