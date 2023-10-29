#ifndef UPDATE
#define UPDATE

#include "consts.h"

const int fileira_de_promocao[LADOS] = {FILEIRA_8, FILEIRA_1};

void adicionar_piece(const int l, const int piece, const int casa);
bool fazer_lance(const int inicio, const int destino);
void remover_piece(const int l, const int p, const int casa);
int fazer_captura(const int inicio, const int destino);
void desfaz_captura();
void desfaz_lance();

#endif