#ifndef BENCH_FENS
#define BENCH_FENS

// Standard bench FEN set (public, derived from the Stockfish bench positions).
// Used by Bench::run_standard_bench to produce deterministic Nodes/NPS output
// that is regex-scrapable by Fastchess / cutechess-cli / OpenBench-style tools.

namespace Bench {
    static const char* bench_fens[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        "3r3k/2r4p/1p1b3r/3TpN1q/2P4p/1P3Q2/pB4PP/5RK1 b - - 0 1",
        "2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4",
        "8/8/8/8/5kp1/P7/8/1K1N4 w - - 0 1",
        "8/8/8/5N2/8/p7/8/2NK3k w - - 0 1",
        "8/3k4/8/8/8/4B3/4KB2/2B5 w - - 0 1",
        "4k3/8/8/8/8/8/8/4K2R w K - 0 1",
        "4k3/8/8/8/8/8/8/R3K3 w Q - 0 1",
        "4k2r/8/8/8/8/8/8/4K3 w k - 0 1",
        "r3k3/8/8/8/8/8/8/4K3 w q - 0 1",
        "4k3/8/8/8/8/8/8/R3K2R w KQ - 0 1",
        "r3k2r/8/8/8/8/8/8/4K3 w kq - 0 1",
        "8/8/8/8/8/8/6k1/4K2R w K - 0 1",
        "8/8/8/8/8/8/1k6/R3K3 w Q - 0 1",
        "4k2r/6K1/8/8/8/8/8/8 w k - 0 1",
        "r3k3/1K6/8/8/8/8/8/8 w q - 0 1",
        "r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1",
        "r3k2r/8/3Q4/8/8/5q2/8/R3K2R b KQkq - 0 1",
        "r7/1rk5/2p5/4p3/4P3/5P2/5NK1/8 w - - 0 1",
        "4k3/2p5/8/3P4/8/8/8/4K3 w - - 0 1",
        "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5Q2/PPPP1PPP/RNB1K1NR w KQkq - 2 3",
        "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4",
        "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq c6 0 2",
        "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq d6 0 2",
        "rnbqkbnr/pppp1ppp/4p3/8/2PP4/8/PP2PPPP/RNBQKBNR b KQkq c3 0 2",
        "rnbqkb1r/pp2pppp/3p1n2/2pP4/4P3/8/PPP2PPP/RNBQKBNR b KQkq - 0 3",
        "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
        "r1bqk2r/pppp1ppp/2nb1n2/4p3/2B1P3/2NP1N2/PPP2PPP/R1BQK2R b KQkq - 0 5",
        "r1bqkbnr/ppp2ppp/2np4/4p3/4P3/2N2N2/PPPP1PPP/R1BQKB1R w KQkq - 0 4",
        "r1bqkbnr/ppp2ppp/2np4/4p3/2B1P3/2N2N2/PPPP1PPP/R1BQK2R b KQkq - 2 4",
        "6k1/1R4pp/8/8/8/8/r5PP/6K1 w - - 0 1",
        "8/8/8/4p3/3kP3/4K3/8/8 w - - 0 1",
        "8/8/3k4/4p3/4P3/4K3/8/8 w - - 0 1",
        "8/8/8/8/3kp3/3p4/3P4/3K4 w - - 0 1",
        "8/6k1/8/8/8/8/1K6/4Q3 w - - 0 1",
        "8/6k1/8/8/8/8/1K6/4R3 w - - 0 1",
        "r1bq1rk1/pp2ppbp/2np1np1/8/3NP3/2N1BP2/PPPQ2PP/2KR1B1R w - - 0 10",
        "4rrk1/pp1n3p/3q2pQ/2p1pb2/2PP4/2P3N1/P2B4/R4RK1 b - - 7 19",
        "r3k2r/p2pqpb1/1n2pnp1/2ppP3/1p2P3/2N2N1p/PPPBBPPP/R3K2R w KQkq - 0 10",
        "r4rk1/1pp1qppp/2np1n2/p1b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        "2rqkbnr/pp2pppp/2n5/3p1b2/3P4/5N2/PP2BPPP/RNBQ1RK1 w k - 4 7",
        "r1b2rk1/pp1nqppp/2p1pn2/3p4/2PP4/2NBPN2/PP3PPP/R1BQ1RK1 w - - 0 9",
        "r1bqk2r/pp1nbppp/2p1pn2/3p4/2PP4/P1NBPN2/1P3PPP/R1BQ1RK1 b kq - 0 8",
        "6k1/p5pp/1p6/2p5/3p4/PP1P4/2P2PPP/6K1 w - - 0 1",
        "8/5pk1/6p1/8/1pP5/pP6/P5P1/5K2 w - - 0 40",
        "8/8/1p2k1p1/3p3p/1P1P1P1P/4KP2/8/8 w - - 0 1"
    };

    static const int bench_fens_count = sizeof(bench_fens) / sizeof(bench_fens[0]);
}

#endif
