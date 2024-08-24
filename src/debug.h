#ifndef DEBUG
#define DEBUG

#include "bitboard.h"

namespace Debug{
    void printBitboard(Bitboard::u64 bitboard);
    void print(const char* msg);
} // namespace Debug


#endif