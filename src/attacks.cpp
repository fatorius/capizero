#include "attacks.h"

#include "gen.h"
#include "bitboard.h"
#include "game.h"
#include "values.h"
#include "estado.h"

static int lva_sq(const Estado &est, int target, int side, Bitboard::u64 occ) {
    if (Gen::bit_esquerda[side ^ 1][target] & occ & est.bit_pieces[side][P])
        return Gen::peao_esquerda[side ^ 1][target];
    if (Gen::bit_direita[side ^ 1][target] & occ & est.bit_pieces[side][P])
        return Gen::peao_direita[side ^ 1][target];

    Bitboard::u64 t;

    t = Gen::bit_moves_cavalo[target] & occ & est.bit_pieces[side][C];
    if (t) return Bitboard::bitscan(t);

    t = Gen::atacantes_bispo_occ(target, occ) & occ & est.bit_pieces[side][B];
    if (t) return Bitboard::bitscan(t);

    t = Gen::atacantes_torre_occ(target, occ) & occ & est.bit_pieces[side][T];
    if (t) return Bitboard::bitscan(t);

    t = (Gen::atacantes_bispo_occ(target, occ) | Gen::atacantes_torre_occ(target, occ))
        & occ & est.bit_pieces[side][D];
    if (t) return Bitboard::bitscan(t);

    t = Gen::bit_moves_rei[target] & occ & est.bit_pieces[side][R];
    if (t) return Bitboard::bitscan(t);

    return -1;
}

int Attacks::menor_atacante(const Estado &est, const int l, const int xl, const int casa){
    if (Gen::bit_esquerda[xl][casa] & est.bit_pieces[l][P]){
        return Gen::peao_esquerda[xl][casa];
    }
    if (Gen::bit_direita[xl][casa] & est.bit_pieces[l][P]){
        return Gen::peao_direita[xl][casa];
    }

    
    Bitboard::u64 t1;

    t1 = Gen::bit_moves_cavalo[casa] & est.bit_pieces[l][C];
    if (t1){
        return Bitboard::bitscan(t1);
    }

    t1 = Gen::bit_moves_bispo[casa] & est.bit_pieces[l][B];
    while (t1){
        int casa2 = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa2];

        if (!(Bitboard::bit_entre[casa2][casa] & est.bit_total)){
            return casa2;
        }
    }

    t1 = Gen::bit_moves_torre[casa] & est.bit_pieces[l][T];
    while (t1){
        int casa2 = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa2];

        if (!(Bitboard::bit_entre[casa2][casa] & est.bit_total)){
            return casa2;
        }
    }

    t1 = Gen::bit_moves_dama[casa] & est.bit_pieces[l][D];
    while(t1){
        int casa2 = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa2];

        if (!(Bitboard::bit_entre[casa2][casa] & est.bit_total)){
            return casa2;
        }
    }

    return -1;
}

bool Attacks::casa_esta_sendo_atacada(const Estado &est, const int l, const int casa){
    if (Gen::bit_peao_defende[l][casa] & est.bit_pieces[l][P]){
        return true;
    }
    if (Gen::bit_moves_cavalo[casa] & est.bit_pieces[l][C]){
        return true;
    }

    int casa2;
    Bitboard::u64 atacantes_deslizantes;
     
    atacantes_deslizantes = Gen::bit_moves_torre[casa] & (est.bit_pieces[l][T] | est.bit_pieces[l][D]);
    atacantes_deslizantes |= (Gen::bit_moves_bispo[casa] & (est.bit_pieces[l][B] | est.bit_pieces[l][D]));

    while (atacantes_deslizantes){
        casa2 = Bitboard::bitscan(atacantes_deslizantes);
        atacantes_deslizantes &= Bitboard::not_mask[casa2];

        if (!(Bitboard::bit_entre[casa2][casa] & est.bit_total)){
            return true;
        }
    } 

    if (Gen::bit_moves_rei[casa] & est.bit_pieces[l][R]){
        return true;
    }

    return false;
}

int Attacks::see(const Estado &est, int from, int to, int captured_piece, int side) {
    int gain[32];
    int d = 0;

    gain[0] = Values::pieces_valor[captured_piece];

    Bitboard::u64 occ = est.bit_total;
    occ &= Bitboard::not_mask[from];

    int prev_piece = est.tabuleiro[from];
    int xside = side ^ 1;

    int next_sq;
    while ((next_sq = lva_sq(est, to, xside, occ)) != -1 && d < 30) {
        d++;
        gain[d] = Values::pieces_valor[prev_piece] - gain[d - 1];
        occ     &= Bitboard::not_mask[next_sq];
        prev_piece = est.tabuleiro[next_sq];
        xside ^= 1;
    }

    while (d > 0) {
        int negated = -gain[d - 1];
        gain[d - 1] = -(negated > gain[d] ? negated : gain[d]);
        d--;
    }

    return gain[0];
}