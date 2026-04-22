#ifndef HASH
#define HASH

#include <cstddef>
#include <cstdint>

#include "bitboard.h"
#include "gen.h"

#define TT_BOUND_NONE  0
#define TT_BOUND_EXACT 1
#define TT_BOUND_LOWER 2
#define TT_BOUND_UPPER 3

namespace Hash{
    struct hashp{
        Bitboard::u64 hashlock;
        uint16_t      inicio;
        uint16_t      dest;
        int32_t       score;
        int8_t        depth;
        uint8_t       bound;
        uint8_t       promove;
        uint8_t       _pad;
    };

    extern Bitboard::u64 chaveAtual, lockAtual;
    extern int hash_inicio, hash_destino;
    extern int hash_score, hash_depth, hash_bound, hash_promove;

    // Number of TT entries per side. Set by iniciar_hash()/realocar_tt().
    extern size_t tt_entries_per_side;

    void iniciar_hash();
    void liberar_memoria();
    void limpar_tt();
    void realocar_tt(int size_mb);
    void adicionar_hash(const int ld, const Gen::lance lc,
                        const int score, const int depth, const int bound,
                        const int promove);
    void adicionar_chave(const int l, const int piece, const int casa);

    void adicionar_pontuacao_de_hash();

    bool hash_lookup(const int l);

    Bitboard::u64 obter_lock();
    Bitboard::u64 obter_chave();
};

#endif