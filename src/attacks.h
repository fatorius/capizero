#ifndef ATTACKS
#define ATTACKS

struct Estado;

namespace Attacks{
    bool casa_esta_sendo_atacada(const Estado &est, const int l, const int casa);
    int menor_atacante(const Estado &est, const int l, const int xl, const int casa);
    int see(const Estado &est, int from, int to, int captured_piece, int side);
};

#endif