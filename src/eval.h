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

    extern const int phase_weights[6];

    // Tudo que depende só da posição dos peões (PST, passados, isolados e
    // escudo do rei) vive na hash de peões de cada Estado. A chave é o par de
    // bitboards de peões, comparado por inteiro: não há colisão possível.
    struct EntradaPeoes{
        uint64_t peoes[LADOS];
        Score score[LADOS];
        Score ala_do_rei[LADOS];
        Score ala_da_dama[LADOS];
    };

    extern Score mobilidade_cavalo[9];
    extern Score mobilidade_bispo[14];
    extern Score mobilidade_torre[15];
    extern Score mobilidade_dama[28];

    extern Score ks_weight_c, ks_weight_b, ks_weight_t, ks_weight_d;

    // Termos de estrutura (valores em values.h), também tunados pelo capi_tuner_adam.
    extern Score passado[8];                         // peão passado, por fileira relativa
    extern Score peao_isolado;                       // por peão isolado
    extern Score coluna_semi_aberta, coluna_aberta;  // por torre
    extern Score par_de_bispos;
    extern Score escudo[6];                          // peão do escudo do rei, ver indice_escudo

    // Índice em escudo[] do peão do lado l em casa, ou -1 se a casa não é do
    // escudo (2ª e 3ª fileiras relativas, colunas a-c ou f-h). Conta só na ala
    // onde o rei está: f-h para o rei em f-h, a-c para o rei em a-c.
    int indice_escudo(int l, int casa);

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