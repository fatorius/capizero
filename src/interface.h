#ifndef INTERFACE
#define INTERFACE

#define TAMANHO_MAXIMO_COMANDO 256

#define COMANDO_DISPLAY_TABULEIRO "d"
#define COMANDO_INVERTER_TABULEIRO "i"
#define COMANDO_COMPUTADOR_CALCULAR "calc"
#define COMANDO_EXIBIR_AJUDA "help"
#define COMANDO_EXIBIR_LANCES "lances"
#define COMANDO_NOVA_PARTIDA "nova"
#define COMANDO_FAZER_LANCE "jogar"
#define COMANDO_ATIVAR_ENGINE "on"
#define COMANDO_DESATIVAR_ENGINE "off"
#define COMANDO_ENCERRAR "quit"
#define COMANDO_CARREGAR_FEN "sb"
#define COMANDO_CONFIGURAR_PROFUNDIDADE "prof"
#define COMANDO_CONFIGURAR_TEMPO "tempo"
#define COMANDO_TROCAR_DE_LADO "troca"
#define COMANDO_DESFAZER_LANCE "desfaz"
#define COMANDO_INICIA_XBOARD "xboard"
#define COMANDO_REALIZAR_PERFT "perft"

extern bool tempo_fixo;
extern bool profundidade_fixa;

extern int tempo_maximo;
extern int profundidade_maxima;

extern int lookup;

extern int tempo_gasto;

extern int lance_inicio, lance_destino;

char *lance_para_string(int inicio, int destino, int promove);
bool ler_comando();
void lance_computador(bool verbose);
void print_lance_algebrico(int a, int b);
int obter_tempo();
void print_resultado();
int converter_lance(char *lnc);

#endif