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

using namespace std;

extern int Interface::lance_inicio, Interface::lance_destino, Interface::lance_promove;

int Interface::tempo_gasto;

int Interface::tempo_maximo, Interface::profundidade_maxima;
bool Interface::tempo_fixo, Interface::profundidade_fixa;

int Interface::computar_tempo_para_lance(int remaining_ms, int inc_ms){
    if (remaining_ms <= 0){
        return 50;
    }

    int budget = remaining_ms / 30 + inc_ms / 2;
    int max_budget = remaining_ms - 50;

    if (max_budget < 50){
        max_budget = 50;
    }

    if (budget < 50){
        budget = 50;
    }
    if (budget > max_budget){
        budget = max_budget;
    }

    return budget;
}

void Interface::obter_pv_uci(char *buf, int max_len, int profundidade){
    Interface::lance_inicio  = Hash::hash_inicio;
    Interface::lance_destino = Hash::hash_destino;
    Interface::lance_promove = Hash::hash_promove;

    int pos = 0;
    buf[0] = '\0';

    for (int x = 0; x < profundidade; x++){
        if (Hash::hash_lookup(Game::lado) == false){
            break;
        }

        char *mv = Interface::lance_para_string(Hash::hash_inicio, Hash::hash_destino, Hash::hash_promove);
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

        Update::fazer_lance(Hash::hash_inicio, Hash::hash_destino, Hash::hash_promove);

        // Search returns VALOR_EMPATE on repetition without storing TT, but the
        // TT may hold a move from an earlier path to the same position. Stop
        // the PV here so output matches what the search actually evaluated.
        if (Game::checar_repeticoes()){
            break;
        }
    }

    while (Game::ply){
        Update::desfaz_lance();
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

void Interface::lance_computador(){
    Search::pensar(false);

    Hash::chaveAtual = Hash::obter_chave();
    Hash::lockAtual = Hash::obter_lock();

    Interface::tempo_gasto = Interface::obter_tempo() - Search::tempo_do_inicio;

    Update::fazer_lance(Hash::hash_inicio, Hash::hash_destino, Hash::hash_promove);

    Eval::atualizar_materiais();

    Game::ply = 0;
    Game::qntt_lances_totais[0] = 0;

    Gen::gerar_lances(Game::lado, Game::xlado);
}

int Interface::converter_lance(char *lnc){
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

    for (i = 0; i < Game::qntt_lances_totais[1]; i++){
        const Gen::lance &m = Gen::lista_de_lances[i];
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

