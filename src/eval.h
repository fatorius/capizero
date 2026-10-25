#ifndef EVAL
#define EVAL

#include <cstdint>

#include "consts.h"

struct Estado;

namespace Eval{
    typedef int32_t Score;

    inline Score make_score(int mg, int eg){
        return (Score)(((uint32_t)eg << 16) + (uint32_t)mg);
    }

    inline int mg_score(Score s){
        return (int16_t)((uint32_t)s & 0xFFFF);
    }

    inline int eg_score(Score s){
        return (int16_t)(((uint32_t)s + 0x8000) >> 16);
    }

    extern Score score_casas[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
    extern int passados[LADOS][CASAS_DO_TABULEIRO];

    extern const int phase_weights[6];

    // Tudo que depende só da posição dos peões (PST, passados, isolados e
    // escudo do rei) vive na hash de peões de cada Estado. A chave é o par de
    // bitboards de peões, comparado por inteiro: não há colisão possível.
    struct EntradaPeoes{
        uint64_t peoes[LADOS];
        Score score[LADOS];
        int16_t ala_do_rei[LADOS];
        int16_t ala_da_dama[LADOS];
    };

    extern Score mobilidade_cavalo[9];
    extern Score mobilidade_bispo[14];
    extern Score mobilidade_torre[15];
    extern Score mobilidade_dama[28];

    extern Score ks_weight_c, ks_weight_b, ks_weight_t, ks_weight_d;

    // King-zone mask the evaluator uses for king-safety pressure (exposed for the tuner).
    uint64_t zona_do_rei(int casa);

    void init_eval_tables();
    void init_king_zone();      // call after Gen::init_lookup_tables()
    void atualizar_materiais(Estado &est);

    // Zera a hash de peões do Estado. Obrigatório após alterar qualquer tabela de avaliação em tempo de execução (tuners).
    void limpar_hash_peoes(Estado &est);

    // Quando false, a hash de peões é ignorada e a estrutura é sempre recalculada. Os tuners desligam, pois alteram as tabelas em tempo de execução.
    extern bool usar_hash_peoes;

    int fase(const Estado &est);

    int avaliar(Estado &est);
};

#endif