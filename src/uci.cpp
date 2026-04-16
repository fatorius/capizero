#include "uci.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bitboard.h"
#include "consts.h"
#include "params.h"
#include "game.h"
#include "gen.h"
#include "update.h"
#include "search.h"
#include "interface.h"
#include "hash.h"
#include "eval.h"

static void send_bestmove(){
    Hash::chaveAtual = Hash::obter_chave();
    Hash::lockAtual = Hash::obter_lock();

    int inicio = Hash::hash_inicio;
    int destino = Hash::hash_destino;

    if (Interface::lance_inicio != 0 || Interface::lance_destino != 0){
        inicio = Interface::lance_inicio;
        destino = Interface::lance_destino;
    }

    int promove = 0;
    if (Bitboard::tabuleiro[inicio] == P &&
        (Consts::linhas[destino] == 0 || Consts::linhas[destino] == 7)){
        promove = D;
    }

    printf("bestmove %s\n", Interface::lance_para_string(inicio, destino, promove));
    fflush(stdout);
}

static void apply_moves_from_tokens(char *tok){
    // tok points at the first move string (or NULL). Consumes via strtok.
    while (tok != NULL){
        Game::ply = 0;
        Game::qntt_lances_totais[0] = 0;
        Gen::gerar_lances(Game::lado, Game::xlado);

        int idx = Interface::converter_lance(tok);
        if (idx < 0){
            printf("info string illegal move in position command: %s\n", tok);
            fflush(stdout);
            return;
        }

        if (!Update::fazer_lance(Gen::lista_de_lances[idx].inicio,
                                 Gen::lista_de_lances[idx].destino)){
            printf("info string illegal move in position command: %s\n", tok);
            fflush(stdout);
            return;
        }

        tok = strtok(NULL, " \t\r\n");
    }
}

static void handle_position(char *args){
    if (args == NULL){
        return;
    }

    char *tok = strtok(args, " \t\r\n");
    if (tok == NULL){
        return;
    }

    if (!strcmp(tok, "startpos")){
        Game::novo_jogo();
        Game::lado_do_computador = VAZIO;

        tok = strtok(NULL, " \t\r\n");
        if (tok != NULL && !strcmp(tok, "moves")){
            tok = strtok(NULL, " \t\r\n");
            apply_moves_from_tokens(tok);
        }
        return;
    }

    if (!strcmp(tok, "fen")){
        char posicao[128]   = {0};
        char lado_a_jogar[8] = {0};
        char roques[8]       = {0};
        char casa_ep[8]      = {0};
        char hm[8]           = {0};
        char fm[8]           = {0};

        char *fields[6] = { posicao, lado_a_jogar, roques, casa_ep, hm, fm };
        int field_sizes[6] = { 127, 7, 7, 7, 7, 7 };

        for (int i = 0; i < 6; i++){
            tok = strtok(NULL, " \t\r\n");
            if (tok == NULL){
                printf("info string bad FEN: missing field %d\n", i);
                fflush(stdout);
                return;
            }
            strncpy(fields[i], tok, field_sizes[i]);
            fields[i][field_sizes[i]] = '\0';
        }

        Update::setar_posicao(posicao, lado_a_jogar, roques, casa_ep, hm, fm);
        Game::lado_do_computador = VAZIO;

        tok = strtok(NULL, " \t\r\n");
        if (tok != NULL && !strcmp(tok, "moves")){
            tok = strtok(NULL, " \t\r\n");
            apply_moves_from_tokens(tok);
        }
    }
}

static void handle_go(char *args){
    int wtime = -1, btime = -1;
    int winc = 0, binc = 0;
    int movetime = -1;
    int depth = -1;
    bool infinite = false;

    if (args != NULL){
        char *tok = strtok(args, " \t\r\n");
        while (tok != NULL){
            if (!strcmp(tok, "wtime")){
                tok = strtok(NULL, " \t\r\n");
                if (tok) wtime = atoi(tok);
            }
            else if (!strcmp(tok, "btime")){
                tok = strtok(NULL, " \t\r\n");
                if (tok) btime = atoi(tok);
            }
            else if (!strcmp(tok, "winc")){
                tok = strtok(NULL, " \t\r\n");
                if (tok) winc = atoi(tok);
            }
            else if (!strcmp(tok, "binc")){
                tok = strtok(NULL, " \t\r\n");
                if (tok) binc = atoi(tok);
            }
            else if (!strcmp(tok, "movetime")){
                tok = strtok(NULL, " \t\r\n");
                if (tok) movetime = atoi(tok);
            }
            else if (!strcmp(tok, "depth")){
                tok = strtok(NULL, " \t\r\n");
                if (tok) depth = atoi(tok);
            }
            else if (!strcmp(tok, "infinite")){
                infinite = true;
            }
            else if (!strcmp(tok, "nodes") ||
                     !strcmp(tok, "mate") ||
                     !strcmp(tok, "movestogo") ||
                     !strcmp(tok, "searchmoves") ||
                     !strcmp(tok, "ponder")){
                // Unsupported; skip the following value to stay in sync.
                tok = strtok(NULL, " \t\r\n");
                continue;
            }
            tok = strtok(NULL, " \t\r\n");
        }
    }

    if (infinite){
        Interface::tempo_maximo = 1 << 25;
        Interface::profundidade_maxima = MAX_PLY;
        Interface::tempo_fixo = false;
        Interface::profundidade_fixa = true;
    }
    else if (depth > 0){
        Interface::tempo_maximo = 1 << 25;
        Interface::profundidade_maxima = depth;
        Interface::tempo_fixo = false;
        Interface::profundidade_fixa = true;
    }
    else if (movetime > 0){
        Interface::tempo_maximo = movetime;
        Interface::profundidade_maxima = MAX_PLY;
        Interface::tempo_fixo = true;
        Interface::profundidade_fixa = false;
    }
    else if (wtime >= 0 || btime >= 0){
        int remaining = (Game::lado == BRANCAS) ? wtime : btime;
        int inc       = (Game::lado == BRANCAS) ? winc  : binc;
        if (remaining < 0) remaining = 1000;
        Interface::tempo_maximo = Interface::computar_tempo_para_lance(remaining, inc);
        Interface::profundidade_maxima = MAX_PLY;
        Interface::tempo_fixo = true;
        Interface::profundidade_fixa = false;
    }
    else{
        Interface::tempo_maximo = 1 << 25;
        Interface::profundidade_maxima = MAX_PLY;
        Interface::tempo_fixo = false;
        Interface::profundidade_fixa = true;
    }

    Search::pensar(true);
    send_bestmove();
}

static void handle_setoption(char *args){
    // "name X [X...] value Y [Y...]" - accept but only a couple of known names.
    // For now we just acknowledge; proper Hash resize / Threads handling TBD.
    if (args == NULL){
        return;
    }
    // Silently accept. Unknown options would generate too much noise otherwise.
}

void Uci::loop(){
    Interface::protocolo_ativo = Interface::PROTO_UCI;

    Game::novo_jogo();
    Game::lado_do_computador = VAZIO;
    Interface::tempo_fixo = false;
    Interface::profundidade_fixa = false;
    Interface::tempo_maximo = 1 << 25;
    Interface::profundidade_maxima = MAX_PLY;

    char linha[4096];

    while (fgets(linha, sizeof(linha), stdin) != NULL){
        // Strip trailing newline.
        size_t len = strlen(linha);
        while (len > 0 && (linha[len-1] == '\n' || linha[len-1] == '\r')){
            linha[--len] = '\0';
        }
        if (len == 0){
            continue;
        }

        // Duplicate for strtok; keep original intact where needed.
        char work[4096];
        strncpy(work, linha, sizeof(work) - 1);
        work[sizeof(work) - 1] = '\0';

        char *cmd = strtok(work, " \t\r\n");
        if (cmd == NULL){
            continue;
        }

        if (!strcmp(cmd, "uci")){
            printf("id name Capizero %s\n", CAPIZERO_VERSION);
            printf("id author Hugo Souza\n");
            printf("option name Hash type spin default 128 min 1 max 4096\n");
            printf("option name Threads type spin default 1 min 1 max 1\n");
            printf("uciok\n");
            fflush(stdout);
        }
        else if (!strcmp(cmd, "isready")){
            printf("readyok\n");
            fflush(stdout);
        }
        else if (!strcmp(cmd, "ucinewgame")){
            Game::novo_jogo();
            Game::lado_do_computador = VAZIO;
            Hash::limpar_tt();
        }
        else if (!strcmp(cmd, "position")){
            char *rest = strtok(NULL, "");
            handle_position(rest);
        }
        else if (!strcmp(cmd, "go")){
            char *rest = strtok(NULL, "");
            handle_go(rest);
        }
        else if (!strcmp(cmd, "stop")){
            // No-op: engine is single-threaded and does not poll stdin mid-search.
        }
        else if (!strcmp(cmd, "quit")){
            return;
        }
        else if (!strcmp(cmd, "setoption")){
            char *rest = strtok(NULL, "");
            handle_setoption(rest);
        }
        else if (!strcmp(cmd, "debug") ||
                 !strcmp(cmd, "register") ||
                 !strcmp(cmd, "ponderhit")){
            // Accept silently.
        }
        else{
            printf("info string unknown command: %s\n", linha);
            fflush(stdout);
        }
    }
}
