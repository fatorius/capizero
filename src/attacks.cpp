#include "attacks.h"

#include "gen.h"
#include "bitboard.h"

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