#include "attacks.h"

#include "gen.h"
#include "bitboard.h"

bool casa_esta_sendo_atacada(const int l, const int casa){
    if (bit_peao_defende[l][casa] & bit_pieces[l][P]){
        return true;
    }
    if (bit_moves_cavalo[casa] & bit_pieces[l][C]){
        return true;
    }

    int casa2;
    u64 atacantes_deslizantes;
     
    atacantes_deslizantes = bit_moves_torre[casa] & (bit_pieces[casa][T] | bit_pieces[casa][D]);
    atacantes_deslizantes |= (bit_moves_bispo[casa] & (bit_pieces[casa][B] | bit_pieces[casa][D]));

    while (atacantes_deslizantes){
        casa2 = bitscan(atacantes_deslizantes);
        atacantes_deslizantes &= not_mask[casa2];

        if (!(bit_entre[casa2][casa] & bit_total)){
            return true;
        }
    } 

    if (bit_moves_rei[casa] & bit_pieces[l][R]){
        return true;
    }

    return false;
}