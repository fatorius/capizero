#ifndef ATTACKS
#define ATTACKS

namespace Attacks{
    bool casa_esta_sendo_atacada(const int l, const int casa);
    int menor_atacante(const int l, const int xl, const int casa);
    // Static Exchange Evaluation. Simulates the full capture exchange on
    // `to`, starting with the piece on `from` (type `captured_piece` already
    // known by the caller). Returns the expected net material gain (positive
    // = winning exchange). Does NOT modify board state.
    int see(int from, int to, int captured_piece, int side);
};

#endif