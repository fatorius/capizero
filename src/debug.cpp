#include "debug.h"

#include <stdio.h>

void Debug::printBitboard(u64 bitboard){
    int bit;

    for (int y = 7; y >= 0; y--){
        for (int x = 0; x < 8; x++){
            bit = (y * 8) + x;
            if (mask[bit] & bitboard){
                printf(" X ");
            }
            else{
                printf(" . ");
            }

            if (bit % 8 == 7){
                printf("\n");
            }
        }
    }
}