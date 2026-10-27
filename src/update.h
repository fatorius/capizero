#ifndef UPDATE
#define UPDATE

#include "consts.h"

struct Estado;

namespace Update{
    const int fileira_de_promocao[LADOS] = {FILEIRA_8, FILEIRA_1};

    void adicionar_piece(Estado &est, const int l, const int piece, const int casa);
    bool fazer_lance(Estado &est, const int inicio, const int destino, const int promove = 0);
    void remover_piece(Estado &est, const int l, const int p, const int casa);
    int fazer_captura(Estado &est, const int inicio, const int destino);
    void setar_posicao(Estado &est, char posicao[80], char lado_a_jogar[1], char roques[4], char casa_en_passant[2], char hm[4], char fm[4]);
    void desfaz_captura(Estado &est);
    void desfaz_lance(Estado &est);
    void fazer_null_move(Estado &est);
    void desfaz_null_move(Estado &est);
};

#endif