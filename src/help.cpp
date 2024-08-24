#include "help.h"
#include "interface.h"

#include <stdio.h>

void Help::exibir_ajuda(){
    printf(COMANDO_DISPLAY_TABULEIRO);
    printf(" - Exibe o tabuleiro.\n");	
    printf(COMANDO_INVERTER_TABULEIRO);
    printf(" - Inverte o tabuleiro.\n");
    printf(COMANDO_COMPUTADOR_CALCULAR);
    printf(" - A engine começa a pensar.\n");
    printf(COMANDO_EXIBIR_AJUDA);
    printf(" - Exibe uma lista de comandos.\n");		
    printf(COMANDO_EXIBIR_LANCES);			
    printf(" - Exibe todos os lances possíveis na posição.\n");
    printf(COMANDO_NOVA_PARTIDA);
    printf(" - Começa um novo jogo.\n");
    printf(COMANDO_ATIVAR_ENGINE);
    printf(" - Liga a engine.\n");
    printf(COMANDO_DESATIVAR_ENGINE);
    printf(" - Desliga a engine.\n");
    printf(COMANDO_CARREGAR_FEN);
    printf(" - Carrega uma posição a partir do FEN.\n");
    printf(COMANDO_CONFIGURAR_PROFUNDIDADE);
    printf(" - Configura o limite de profundidade da pesquisa.\n");
    printf(COMANDO_CONFIGURAR_TEMPO);
    printf(" - Configura o tempo de pesquisa da engine.\n");
    printf(COMANDO_TROCAR_DE_LADO);
    printf(" - Muda os lados dos jogadores.\n");
    printf(COMANDO_ENCERRAR);
    printf(" - Encerra o programa.\n");
    printf(COMANDO_DESFAZER_LANCE);
    printf(" - Desfaz o último lance.\n");
    printf(COMANDO_INICIA_XBOARD);
    printf(" - Inicia xboard.\n");	
}