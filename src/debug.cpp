#include "debug.h"

#include <stdio.h>
#include <iostream>

void Debug::printBitboard(Bitboard::u64 bitboard){
    printf("======================\n");
    
    int bit;

    for (int y = 7; y >= 0; y--){
        for (int x = 0; x < 8; x++){
            bit = (y * 8) + x;
            if (Bitboard::mask[bit] & bitboard){
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

    
    printf("======================\n");
}

void Debug::print(const char* msg){
    std::cout<<msg<<std::endl;
}