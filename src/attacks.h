#ifndef ATTACKS
#define ATTACKS

namespace Attacks{
    bool casa_esta_sendo_atacada(const int l, const int casa);
    int menor_atacante(const int l, const int xl, const int casa);
    int see(int from, int to, int captured_piece, int side);
};

#endif