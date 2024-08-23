#ifndef HASH
#define HASH

#include "bitboard.h"
#include "gen.h"

namespace Hash{
    struct hashp{
        Bitboard::u64 hashlock;
        int inicio;
        int dest;
        int num;
    };

    extern Bitboard::u64 chaveAtual, lockAtual;
    extern int hash_inicio, hash_destino;

    void iniciar_hash();
    void liberar_memoria();
    void adicionar_hash(const int ld, const Gen::lance lc);
    void adicionar_chave(const int l, const int piece, const int casa);

    void adicionar_pontuacao_de_hash();

    bool hash_lookup(const int l);

    Bitboard::u64 obter_lock();
    Bitboard::u64 obter_chave();
};

#endif