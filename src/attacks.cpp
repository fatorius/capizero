#include "attacks.h"

#include "gen.h"
#include "bitboard.h"
#include "game.h"
#include "values.h"

// Returns the square of the least-valuable piece of `side` that attacks
// `target` given occupancy `occ`. Returns -1 if no attacker. Handles
// X-ray reveals because the caller removes each found attacker from `occ`
// before the next call, and the magic lookup uses the updated `occ`.
static int lva_sq(int target, int side, Bitboard::u64 occ) {
    // Pawns: bit_esquerda/bit_direita are single-bit boards for the square
    // that a pawn of `side` would occupy to attack `target` from that side.
    if (Gen::bit_esquerda[side ^ 1][target] & occ & Bitboard::bit_pieces[side][P])
        return Gen::peao_esquerda[side ^ 1][target];
    if (Gen::bit_direita[side ^ 1][target] & occ & Bitboard::bit_pieces[side][P])
        return Gen::peao_direita[side ^ 1][target];

    Bitboard::u64 t;

    // Knights
    t = Gen::bit_moves_cavalo[target] & occ & Bitboard::bit_pieces[side][C];
    if (t) return Bitboard::bitscan(t);

    // Bishops (not queens — queens come later so bishops are preferred by LVA)
    t = Gen::atacantes_bispo_occ(target, occ) & occ & Bitboard::bit_pieces[side][B];
    if (t) return Bitboard::bitscan(t);

    // Rooks (not queens)
    t = Gen::atacantes_torre_occ(target, occ) & occ & Bitboard::bit_pieces[side][T];
    if (t) return Bitboard::bitscan(t);

    // Queens (any ray)
    t = (Gen::atacantes_bispo_occ(target, occ) | Gen::atacantes_torre_occ(target, occ))
        & occ & Bitboard::bit_pieces[side][D];
    if (t) return Bitboard::bitscan(t);

    // King
    t = Gen::bit_moves_rei[target] & occ & Bitboard::bit_pieces[side][R];
    if (t) return Bitboard::bitscan(t);

    return -1;
}

int Attacks::menor_atacante(const int l, const int xl, const int casa){
    // Peões atacantes automaticamente serão os menores atacantes
    if (Gen::bit_esquerda[xl][casa] & Bitboard::bit_pieces[l][P]){
        return Gen::peao_esquerda[xl][casa];
    }
    if (Gen::bit_direita[xl][casa] & Bitboard::bit_pieces[l][P]){
        return Gen::peao_direita[xl][casa];
    }

    
    Bitboard::u64 t1;

    t1 = Gen::bit_moves_cavalo[casa] & Bitboard::bit_pieces[l][C];
    if (t1){
        return Bitboard::bitscan(t1);
    }

    t1 = Gen::bit_moves_bispo[casa] & Bitboard::bit_pieces[l][B];
    while (t1){
        int casa2 = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa2];

        if (!(Bitboard::bit_entre[casa2][casa] & Bitboard::bit_total)){
            return casa2;
        }
    }

    t1 = Gen::bit_moves_torre[casa] & Bitboard::bit_pieces[l][T];
    while (t1){
        int casa2 = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa2];

        if (!(Bitboard::bit_entre[casa2][casa] & Bitboard::bit_total)){
            return casa2;
        }
    }

    t1 = Gen::bit_moves_dama[casa] & Bitboard::bit_pieces[l][D];
    while(t1){
        int casa2 = Bitboard::bitscan(t1);
        t1 &= Bitboard::not_mask[casa2];

        if (!(Bitboard::bit_entre[casa2][casa] & Bitboard::bit_total)){
            return casa2;
        }
    }

    return -1;
}

bool Attacks::casa_esta_sendo_atacada(const int l, const int casa){
    if (Gen::bit_peao_defende[l][casa] & Bitboard::bit_pieces[l][P]){
        return true;
    }
    if (Gen::bit_moves_cavalo[casa] & Bitboard::bit_pieces[l][C]){
        return true;
    }

    int casa2;
    Bitboard::u64 atacantes_deslizantes;
     
    atacantes_deslizantes = Gen::bit_moves_torre[casa] & (Bitboard::bit_pieces[l][T] | Bitboard::bit_pieces[l][D]);
    atacantes_deslizantes |= (Gen::bit_moves_bispo[casa] & (Bitboard::bit_pieces[l][B] | Bitboard::bit_pieces[l][D]));

    while (atacantes_deslizantes){
        casa2 = Bitboard::bitscan(atacantes_deslizantes);
        atacantes_deslizantes &= Bitboard::not_mask[casa2];

        if (!(Bitboard::bit_entre[casa2][casa] & Bitboard::bit_total)){
            return true;
        }
    } 

    if (Gen::bit_moves_rei[casa] & Bitboard::bit_pieces[l][R]){
        return true;
    }

    return false;
}

int Attacks::see(int from, int to, int captured_piece, int side) {
    // gain[d] = expected net material gain for the side making the d-th
    // capture, given optimal play by both sides from ply d onwards.
    //
    // gain[0] = value of the initially captured piece.
    // gain[d] = value[prev_attacker] - gain[d-1]
    //   where prev_attacker is the piece placed on `to` by ply d-1 (i.e.,
    //   the piece the current side is about to retake).
    //
    // Minimax backprop: gain[d-1] = -max(-gain[d-1], gain[d])
    // Each side can choose not to recapture if doing so would lose material.
    int gain[32];
    int d = 0;

    gain[0] = Values::pieces_valor[captured_piece];

    Bitboard::u64 occ = Bitboard::bit_total;
    occ &= Bitboard::not_mask[from]; // remove initial attacker from the occupancy

    int prev_piece = Bitboard::tabuleiro[from]; // piece now "on" the target square
    int xside = side ^ 1;                       // opponent recaptures first

    int next_sq;
    while ((next_sq = lva_sq(to, xside, occ)) != -1 && d < 30) {
        d++;
        gain[d] = Values::pieces_valor[prev_piece] - gain[d - 1];
        occ     &= Bitboard::not_mask[next_sq];
        prev_piece = Bitboard::tabuleiro[next_sq];
        xside ^= 1;
    }

    // Minimax: each side plays optimally — they only recapture if it's good.
    while (d > 0) {
        int negated = -gain[d - 1];
        gain[d - 1] = -(negated > gain[d] ? negated : gain[d]);
        d--;
    }

    return gain[0];
}