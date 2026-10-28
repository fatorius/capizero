#include "interface.h"

#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>
#include <chrono>

#include "game.h"
#include "gen.h"
#include "update.h"
#include "search.h"
#include "hash.h"
#include "eval.h"
#include "estado.h"

using namespace std;

int Interface::tempo_gasto;

int Interface::tempo_maximo, Interface::profundidade_maxima;
bool Interface::tempo_fixo, Interface::profundidade_fixa;

int Interface::computar_tempo_para_lance(int remaining_ms, int inc_ms){
    if (remaining_ms <= 0){
        return TEMPO_MINIMO_LANCE;
    }

    int budget = remaining_ms / 30 + inc_ms / 2;
    int max_budget = remaining_ms - TEMPO_RESERVA;

    if (max_budget < TEMPO_MINIMO_LANCE){
        max_budget = TEMPO_MINIMO_LANCE;
    }

    if (budget < TEMPO_MINIMO_LANCE){
        budget = TEMPO_MINIMO_LANCE;
    }
    if (budget > max_budget){
        budget = max_budget;
    }

    return budget;
}

void Interface::obter_pv_uci(Estado &est, char *buf, int max_len, int profundidade){
    int pos = 0;
    buf[0] = '\0';

    for (int x = 0; x < profundidade; x++){
        int inicio, destino, promove;

        if (x == 0){
            inicio  = est.lance_escolhido.inicio;
            destino = est.lance_escolhido.destino;
            promove = est.lance_escolhido.promove;
        }
        else{
            if (!Hash::hash_lookup(est, est.lado)){
                break;
            }
            inicio  = est.hash_inicio;
            destino = est.hash_destino;
            promove = est.hash_promove;
        }

        if (inicio == destino || !Gen::lance_pseudo_legal(est, inicio, destino, promove)){
            break;
        }

        if (!Update::fazer_lance(est, inicio, destino, promove)){
            break;
        }

        char *mv = Interface::lance_para_string(inicio, destino, promove);
        int len = strlen(mv);
        if (pos + len + 2 >= max_len){
            break;
        }
        if (pos > 0){
            buf[pos++] = ' ';
        }
        memcpy(buf + pos, mv, len);
        pos += len;
        buf[pos] = '\0';

        // Search returns VALOR_EMPATE on repetition without storing TT, but the
        // TT may hold a move from an earlier path to the same position. Stop
        // the PV here so output matches what the search actually evaluated.
        if (Game::checar_repeticoes(est)){
            break;
        }
    }

    while (est.ply){
        Update::desfaz_lance(est);
    }
}

int Interface::obter_tempo(){
    return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

char *Interface::lance_para_string(int inicio, int destino, int promove){
    static char str[6];

    char c;

    if (promove > P){
        switch (promove){
        case C:
            c = 'n';
            break;
        case B:
            c = 'b';
            break;
        case R:
            c = 'r';
            break;
        default:
            c = 'q';
            break;
        }
    
        snprintf(str, sizeof(str), "%c%d%c%d%c",
            Consts::colunas[inicio] + 'a',
            Consts::linhas[inicio] + 1,
            Consts::colunas[destino] + 'a',
            Consts::linhas[destino] + 1,
            c);
    }
    else{
        snprintf(str, sizeof(str), "%c%d%c%d",
			Consts::colunas[inicio] + 'a',
			Consts::linhas[inicio] + 1,
			Consts::colunas[destino] + 'a',
			Consts::linhas[destino] + 1);
    }

    return str;
}

void Interface::lance_computador(Estado &est){
    Search::pensar(est, false);

    est.chaveAtual = Hash::obter_chave(est);
    est.lockAtual = Hash::obter_lock(est);

    Interface::tempo_gasto = Interface::obter_tempo() - Search::tempo_do_inicio;

    Update::fazer_lance(est, est.lance_escolhido.inicio, est.lance_escolhido.destino, est.lance_escolhido.promove);

    Eval::atualizar_materiais(est);

    est.ply = 0;
    est.qntt_lances_totais[0] = 0;

    Gen::gerar_lances(est, est.lado, est.xlado);
}

int Interface::converter_lance(const Estado &est, char *lnc){
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
    destino += ((lnc[3] - '0') - 1) * 8;

    // Parse the optional promotion char. 0 = unspecified (UCI tolerance → queen).
    int pp = 0;
    if      (lnc[4] == 'q' || lnc[4] == 'Q') pp = D;
    else if (lnc[4] == 'n' || lnc[4] == 'N') pp = C;
    else if (lnc[4] == 'b' || lnc[4] == 'B') pp = B;
    else if (lnc[4] == 'r' || lnc[4] == 'R') pp = T;

    for (i = 0; i < est.qntt_lances_totais[1]; i++){
        const Gen::lance &m = est.lista_de_lances[i];
        if (m.inicio != inicio || m.destino != destino){
            continue;
        }
        if (m.promove == 0){
            return i;                                 // non-promotion move
        }
        if (pp == 0 && m.promove == D){
            return i;                                 // UCI tolerance: default queen
        }
        if (pp != 0 && m.promove == pp){
            return i;                                 // explicit match
        }
    }

    return -1;
}

