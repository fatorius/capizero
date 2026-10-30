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
    // Aloca a TT no tamanho padrão se nenhum setoption Hash chegou antes.
    void garantir_tt();
    void adicionar_hash(const Estado &est, const int ld, const Gen::lance lc,
                        const int score, const int depth, const int bound,
                        const int promove);
    void adicionar_chave(Estado &est, const int l, const int piece, const int casa);

    // Coluna do peão que acabou de avançar duas casas, se um peão do lado a
    // jogar está ao lado dele (a mesma condição de Gen::gerar_en_passant);
    // -1 se não há captura en passant.
    int coluna_en_passant(const Estado &est);

    // Depois de um lance: troca na chave os direitos de roque (roque_antes ->
    // est.roque) e a coluna de en passant (ep_antes -> ep; -1 = nenhuma).
    void atualizar_roque_ep(Estado &est, const int roque_antes, const int ep_antes, const int ep);

    void adicionar_pontuacao_de_hash(Estado &est);

    bool hash_lookup(Estado &est, const int l);

    // Chave e lock calculados do zero: peças, direitos de roque e en passant.
    Bitboard::u64 obter_lock(const Estado &est);
    Bitboard::u64 obter_chave(const Estado &est);
};

#endif