#ifndef INTERFACE
#define INTERFACE

#define TAMANHO_MAXIMO_COMANDO 256

#define COMANDO_DISPLAY_TABULEIRO "d"
#define COMANDO_INVERTER_TABULEIRO "f"
#define COMANDO_COMPUTADOR_CALCULAR "go"
#define COMANDO_EXIBIR_AJUDA "help"
#define COMANDO_EXIBIR_LANCES "moves"
#define COMANDO_NOVA_PARTIDA "new"
#define COMANDO_FAZER_LANCE "p"
#define COMANDO_ATIVAR_ENGINE "on"
#define COMANDO_DESATIVAR_ENGINE "off"
#define COMANDO_ENCERRAR "quit"
#define COMANDO_CARREGAR_FEN "sb"
#define COMANDO_CONFIGURAR_PROFUNDIDADE "sd"
#define COMANDO_CONFIGURAR_TEMPO "st"
#define COMANDO_TROCAR_DE_LADO "sw"
#define COMANDO_DESFAZER_LANCE "undo"
#define COMANDO_INICIA_XBOARD "xboard"

extern int tempo_fixo;
extern int profundidade_fixa;

extern int lookup;

extern int lance_inicio, lance_destino;

bool ler_comando();
void lance_computador();
void print_lance_algebrico(int a, int b);
int obter_tempo();

#endif