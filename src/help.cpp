#include "help.h"

#include <stdio.h>

void exibir_ajuda(){
    printf("d - Exibe o tabuleiro.\n");	
    printf("f - Inverte o tabuleiro.\n");
    printf("go - A engine começa a pensar.\n");
    printf("help - Exibe uma lista de comandos.\n");					
    printf("moves - Exibe todos os lances possíveis na posição.\n");
    printf("new - Começa um novo jogo.\n");
    printf("on - Liga a engine.\n");
    printf("off - Desliga a engine.\n");
    printf("sb - Carrega uma posição a partir do FEN.\n");
    printf("sd - Configura o limite de profundidade da pesquisa.\n");
    printf("st - Configura o tempo de pesquisa da engine.\n");
    printf("sw - Muda os lados dos jogadores.\n");
    printf("quit - Encerra o programa.\n");
    printf("undo - Desfaz o último lance.\n");
    printf("xboard - Inicia xboard.\n");	
}