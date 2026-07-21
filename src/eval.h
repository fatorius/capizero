#ifndef EVAL
#define EVAL

#include <cstdint>

#include "consts.h"

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

    extern int peao_mat[2];
    extern int piece_mat[2];

    extern int fase_valor;
    extern const int phase_weights[6];

    extern Score mobilidade_cavalo[9];
    extern Score mobilidade_bispo[14];
    extern Score mobilidade_torre[15];
    extern Score mobilidade_dama[28];

    extern Score ks_weight_c, ks_weight_b, ks_weight_t, ks_weight_d;

    void init_eval_tables();
    void atualizar_materiais();

    int fase();

    int avaliar();
};

#endif