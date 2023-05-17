#include "xboard.h"

#include <stdio.h>
#include <string.h>
#include <iostream>

#include "game.h"
#include "gen.h"
#include "update.h"

using namespace std;

int lance_usuario;

int converter_lance(char *lnc){
    int inicio, destino, i;

    if (
        lnc[0] < 'a' || lnc[0] > 'h' ||
        lnc[1] < '0' || lnc[1] > '9' ||
        lnc[2] < 'a' || lnc[2] > 'h' ||
        lnc[3] < '0' || lnc[3] > '9'
        ){
        return -1;
    }
    
    inicio = lnc[0] - 'a';
    inicio += ((lnc[1] - '0') - 1) * 8;
    destino = lnc[2] - 'a';
    destino += ((lnc[2] - '0') - 1) * 8;

    for (i = 0; i < primeiro_lance[1]; i++){
        if (lista_de_lances[i].inicio == inicio && lista_de_lances[i].destino == destino){
            if (lnc[4] == 'n' || lnc[4]=='N'){
                lista_de_lances[i].promove = C;
            }
            if (lnc[4] == 'b' || lnc[4]=='B'){
                lista_de_lances[i].promove = B;
            }
            if (lnc[4] == 'r' || lnc[4]=='R'){
                lista_de_lances[i].promove = T;
            }

            return i;
        }
    }

    return -1;
}

void processar_lance_do_usuario(char lnc[TAMANHO_MAXIMO_COMANDO]){

    ply = 0;
    primeiro_lance[0] = 0;
    gerar_lances(lado, xlado);

    lance_usuario = converter_lance(lnc);

    if (lance_usuario == -1 || fazer_lance(lista_de_lances[lance_usuario].inicio, lista_de_lances[lance_usuario].destino)){
        cout<<"Comando / Lance inválido"<<endl;
        cout<<"Digite 'help' para exibir uma lista de comandos válidos ou"<<endl;
        cout<<"Digite 'moves' para exibir uma lista de lances legais"<<endl<<endl;
        return;
    }

    // atualiza peça promovida
    if (lista_de_lances[hply - 1].promove > P && (colunas[lista_de_lances[lance_usuario].destino] == FILEIRA_1 || colunas[lista_de_lances[lance_usuario].destino] == FILEIRA_8)){
        remover_piece(xlado, D, lista_de_lances[lance_usuario].destino);

        if (lnc[PROMOCAO] == 'n' || lnc[PROMOCAO] == 'N'){
            adicionar_piece(xlado, C, lista_de_lances[lance_usuario].destino);
        }
        else if (lnc[PROMOCAO] == 'b' || lnc[PROMOCAO] == 'B'){
            adicionar_piece(xlado, B, lista_de_lances[lance_usuario].destino);
        }
        else if (lnc[PROMOCAO] == 'r' || lnc[PROMOCAO] == 'R'){
            adicionar_piece(xlado, T, lista_de_lances[lance_usuario].destino);
        }
        else{
            adicionar_piece(xlado, D, lista_de_lances[lance_usuario].destino);
        }
    }

    return;
}

bool ler_comando(){
    char cmd[TAMANHO_MAXIMO_COMANDO];

    printf(">> ");

    if (scanf("%s", cmd) == EOF){
        return false;
    }
    
    else if (!strcmp(cmd, COMANDO_DISPLAY_TABULEIRO)){

    }
    else if (!strcmp(cmd, COMANDO_INVERTER_TABULEIRO)){

    }
    else if (!strcmp(cmd, COMANDO_COMPUTADOR_CALCULAR)){

    }
    else if (!strcmp(cmd, COMANDO_EXIBIR_AJUDA)){

    }
    else if (!strcmp(cmd, COMANDO_EXIBIR_LANCES)){

    }
    else if (!strcmp(cmd, COMANDO_NOVA_PARTIDA)){

    }
    else if (!strcmp(cmd, COMANDO_ATIVAR_ENGINE)){

    }
    else if (!strcmp(cmd, COMANDO_DESATIVAR_ENGINE)){

    }
    else if (!strcmp(cmd, COMANDO_ENCERRAR)){
        return false;
    }
    else if (!strcmp(cmd, COMANDO_CARREGAR_FEN)){

    }
    else if (!strcmp(cmd, COMANDO_CONFIGURAR_PROFUNDIDADE)){

    }
    else if (!strcmp(cmd, COMANDO_CONFIGURAR_TEMPO)){

    }
    else if (!strcmp(cmd, COMANDO_TROCAR_DE_LADO)){

    }
    else if (!strcmp(cmd, COMANDO_DESFAZER_LANCE)){

    }
    else{
        processar_lance_do_usuario(cmd);
    }

    return true;
}