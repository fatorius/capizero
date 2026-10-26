#ifndef HASH
#define HASH

#include <cstddef>
#include <cstdint>
#include <atomic>

#include "bitboard.h"
#include "gen.h"

#define TT_BOUND_NONE  0
#define TT_BOUND_EXACT 1
#define TT_BOUND_LOWER 2
#define TT_BOUND_UPPER 3

struct Estado;

namespace Hash{
    struct hashp{
        std::atomic<Bitboard::u64> chave_xor;
        std::atomic<Bitboard::u64> dados;
    };

    struct EntradaTT{
        int inicio;
        int destino;
        int score;
        int depth;
        int bound;
        int promove;
    };

    Bitboard::u64 empacotar(const EntradaTT &e);
    EntradaTT desempacotar(Bitboard::u64 dados);

    hashp *entrada(const Estado &est, const int l);

    // Number of TT entries per side. Set by iniciar_hash()/realocar_tt().
    extern size_t tt_entries_per_side;

    void iniciar_hash();
    void liberar_memoria();
    void limpar_tt();
    void realocar_tt(int size_mb);
    void adicionar_hash(const Estado &est, const int ld, const Gen::lance lc,
                        const int score, const int depth, const int bound,
                        const int promove);
    void adicionar_chave(Estado &est, const int l, const int piece, const int casa);

    void adicionar_pontuacao_de_hash(Estado &est);

    bool hash_lookup(Estado &est, const int l);

    Bitboard::u64 obter_lock(const Estado &est);
    Bitboard::u64 obter_chave(const Estado &est);
};

#endif