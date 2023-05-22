#ifndef UPDATE
#define UPDATE

void adicionar_piece(const int l, const int piece, const int casa);
bool fazer_lance(const int inicio, const int destino);
void remover_piece(const int l, const int p, const int casa);
int fazer_captura(const int inicio, const int destino);
void desfaz_captura();
void desfaz_lance();

#endif