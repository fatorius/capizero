#include "interface.h"

#include <cstdio>
#include <stdio.h>
#include <string.h>
#include <sys/timeb.h>
#include <chrono>

#include "game.h"
#include "gen.h"
#include "update.h"
#include "xboard.h"
#include "help.h"
#include "search.h"
#include "hash.h"
#include "eval.h"
#include "attacks.h"

using namespace std;

int no_lances = 0;

int lookup;

int nps;

int lance_usuario;
int tabuleiro_invertido = 0;

extern int lance_inicio, lance_destino;

int tempo_gasto;

int tempo_maximo;
int profundidade_maxima;

bool tempo_fixo, profundidade_fixa;

int profundidade_perft;

void exibir_melhor_linha(int profundidade){
     lance_inicio = hash_inicio;
     lance_destino = hash_destino;

     for (int x = 0; x < profundidade; x++){
        if (hash_lookup(Game::lado) == false){
            break;
        }

        printf(" ");
        print_lance_algebrico(hash_inicio, hash_destino);
        fazer_lance(hash_inicio, hash_destino);
     }

     while (Game::ply){
        desfaz_lance();
     }
}

void converter_casa_para_algebrico(int a){
    if(a < 0 || a > 63){
        return;
    }  

	char c[2]="a";
    c[0] =  96 + 1 + Consts::colunas[a];
	
    printf("%s%d", c, Consts::linhas[a]+1);
}

void print_lance_algebrico(int a, int b){
    converter_casa_para_algebrico(a);
    converter_casa_para_algebrico(b);
}

int obter_tempo(){
    return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

void display_tabuleiro() {
    printf("\n");

    int i;
    int c;

    for (int j = 0; j < 64; ++j) {
        if (tabuleiro_invertido == 0){
            i = Consts::flip[j];
        }
        else{
            i = 63 - Consts::flip[j];
        }
        
        c = VAZIO;
        
        if (Bitboard::bit_lados[BRANCAS] & Bitboard::mask[i]){ 
            c = BRANCAS;
        }
        if (Bitboard::bit_lados[PRETAS] & Bitboard::mask[i]){
            c = PRETAS;
        }

        switch (c) {
            case VAZIO:
                printf("  ");
                break;
            case BRANCAS:
                printf(" %c", Consts::piece_char[Bitboard::tabuleiro[i]]);
                break;
            case PRETAS:
                printf(" %c", Consts::piece_char[Bitboard::tabuleiro[i]] + ('a' - 'A'));
                break;
            default:
                printf(" %d.", c);
                break;
        }

        if (tabuleiro_invertido == 0) {
            if ((j + 1) % 8 == 0 && j != 63){
                printf("\n");
            }
        } else {
            if ((j + 1) % 8 == 0 && Consts::colunas[i] != COLUNA_H){
                printf("\n");
            }
        }
    }

    printf("\n\n");
}

char *lance_para_string(int inicio, int destino, int promove){
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
    
        sprintf(str, "%c%d%c%d%c",
            Consts::colunas[inicio] + 'a',
            Consts::linhas[inicio] + 1,
            Consts::colunas[destino] + 'a',
            Consts::linhas[destino] + 1,
            c);
    }
    else{
        sprintf(str, "%c%d%c%d",
			Consts::colunas[inicio] + 'a',
			Consts::linhas[inicio] + 1,
			Consts::colunas[destino] + 'a',
			Consts::linhas[destino] + 1);
    }

    return str;
}

int repeticoes(){
    int r = 0;

    for (int i = Game::hply; i >= Game::hply-Game::cinquenta; i -= 2){
        if (Game::lista_do_jogo[i].hash == chaveAtual && Game::lista_do_jogo[i].lock == lockAtual){
            r++;
        }
    }

    return r;
}

void print_resultado(){
    int i;
    bool existem_lances_legais = false;

    Eval::atualizar_materiais();
    Gen::gerar_lances(Game::lado, Game::xlado);

    for (i = 0; i < Game::qntt_lances_totais[1]; ++i){
        if (fazer_lance(Gen::lista_de_lances[i].inicio, Gen::lista_de_lances[i].destino)){
            desfaz_lance();
            existem_lances_legais = true;
            break;
        }
    }

    if (Eval::peao_mat[BRANCAS] == 0 && Eval::peao_mat[PRETAS] == 0 && Eval::piece_mat[BRANCAS] <= VALOR_BISPO && Eval::piece_mat[PRETAS] <= VALOR_BISPO){
        printf("1/2 - 1/2 {Material insuficiente} \n");

        Game::novo_jogo();

        Game::lado_do_computador = VAZIO;

        return;
    } 
    else if (i == Game::qntt_lances_totais[1] && !existem_lances_legais){
        display_tabuleiro();

        printf("Fim do jogo \n");

        if (Attacks::casa_esta_sendo_atacada(Game::xlado, Bitboard::bitscan(Bitboard::bit_pieces[Game::lado][R]))){
            if (Game::lado == BRANCAS){
                printf("0-1 {Pretas dao xeque-mate} \n");
            }
            else{
                printf("1-0 {Brancas dao xeque-mate} \n");
            }
        }
        else{
            printf("1/2 - 1/2 {Empate por afogamento} \n");
        }

        Game::novo_jogo();
        Game::lado_do_computador = VAZIO;
    }
    else if (repeticoes() >= 3){
        printf("1/2 - 1/2 - {Empate por repeticao} \n");
        
        Game::novo_jogo();
        Game::lado_do_computador = VAZIO;
    }
    else if (Game::cinquenta >= 100){
        printf("1/2 - 1/2 {Empate por cinquenta lances}");

        Game::novo_jogo();
        Game::lado_do_computador = VAZIO;
    }
}

void lance_computador(bool verbose){
    Game::jogador[Game::lado] = 1;

    pensar(verbose);

    no_lances++;

    chaveAtual = obter_chave();
    lockAtual = obter_lock();

    lookup = hash_lookup(Game::lado);

    tempo_gasto = obter_tempo() - tempo_do_inicio;

    if (verbose){
        if (lance_inicio != 0 || lance_destino != 0){
            hash_inicio = lance_inicio;
            hash_destino = lance_destino;
        }
        else{
            printf("(Sem lances legais) \n");
            Game::lado_do_computador = VAZIO;

            display_tabuleiro();

            Gen::gerar_lances(Game::lado, Game::xlado);

            return;
        }
    }
    
    fazer_lance(hash_inicio, hash_destino);

    Eval::atualizar_materiais();

    Game::ply = 0;
    Game::qntt_lances_totais[0] = 0;

    Gen::gerar_lances(Game::lado, Game::xlado);

    if (verbose){
        printf("Lance do computador: %s \n", lance_para_string(hash_inicio, hash_destino, 0));
        printf("\n");

        printf("\nTempo gasto: %d ms \n", tempo_gasto);

        if (tempo_gasto > 0){
            nps = (double) lances_avaliados / (double) tempo_gasto;
            nps *= 1000.0;
        }
        else{
            nps = 0;
        }

        printf("\nLances por segundo: %d \n", (int) nps);

        print_resultado();

        printf("Lance ");
        printf("%d", no_lances++);

        display_tabuleiro();
    }
}

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
    destino += ((lnc[3] - '0') - 1) * 8;

    for (i = 0; i < Game::qntt_lances_totais[1]; i++){
        if (Gen::lista_de_lances[i].inicio == inicio && Gen::lista_de_lances[i].destino == destino){
            if (lnc[4] == 'n' || lnc[4]=='N'){
                Gen::lista_de_lances[i].promove = C;
            }
            if (lnc[4] == 'b' || lnc[4]=='B'){
                Gen::lista_de_lances[i].promove = B;
            }
            if (lnc[4] == 'r' || lnc[4]=='R'){
                Gen::lista_de_lances[i].promove = T;
            }

            return i;
        }
    }

    return -1;
}

void processar_lance_do_usuario(char lnc[TAMANHO_MAXIMO_COMANDO]){
    Game::ply = 0;
    Game::qntt_lances_totais[0] = 0;
    Gen::gerar_lances(Game::lado, Game::xlado);

    lance_usuario = converter_lance(lnc);

    if (lance_usuario == -1 || !fazer_lance(Gen::lista_de_lances[lance_usuario].inicio, Gen::lista_de_lances[lance_usuario].destino)){
        printf("Comando / Lance inválido\n");
        printf("Digite 'ajuda' para exibir uma lista de comandos válidos ou\n");
        printf("Digite 'lances' para exibir uma lista de lances legais\n");
        printf("\n");
        return;
    }

    // atualiza peça promovida
    if (Gen::lista_de_lances[Game::hply - 1].promove > P && (Consts::colunas[Gen::lista_de_lances[lance_usuario].destino] == FILEIRA_1 || Consts::colunas[Gen::lista_de_lances[lance_usuario].destino] == FILEIRA_8)){
        remover_piece(Game::xlado, D, Gen::lista_de_lances[lance_usuario].destino);

        if (lnc[PROMOCAO] == 'n' || lnc[PROMOCAO] == 'N'){
            adicionar_piece(Game::xlado, C, Gen::lista_de_lances[lance_usuario].destino);
        }
        else if (lnc[PROMOCAO] == 'b' || lnc[PROMOCAO] == 'B'){
            adicionar_piece(Game::xlado, B, Gen::lista_de_lances[lance_usuario].destino);
        }
        else if (lnc[PROMOCAO] == 'r' || lnc[PROMOCAO] == 'R'){
            adicionar_piece(Game::xlado, T, Gen::lista_de_lances[lance_usuario].destino);
        }
        else{
            adicionar_piece(Game::xlado, D, Gen::lista_de_lances[lance_usuario].destino);
        }
    }

    return;
}

bool ler_comando(){
    fflush(stdout);
    
    char cmd[TAMANHO_MAXIMO_COMANDO];

    if (scanf("%s", cmd) == EOF){
        return false;
    }
    
    else if (!strcmp(cmd, COMANDO_DISPLAY_TABULEIRO)){
        display_tabuleiro();
    }
    else if (!strcmp(cmd, COMANDO_INVERTER_TABULEIRO)){
        tabuleiro_invertido = 1 - tabuleiro_invertido;
    }
    else if (!strcmp(cmd, COMANDO_EXIBIR_AJUDA)){
        exibir_ajuda();
    }
    else if (!strcmp(cmd, COMANDO_EXIBIR_LANCES)){
        printf("Lances legais: \n");

        for (int i = 0; i < Game::qntt_lances_totais[1]; i++){
            printf("%s", lance_para_string(Gen::lista_de_lances[i].inicio, Gen::lista_de_lances[i].destino, Gen::lista_de_lances[i].promove));
            printf("\n");
        }
    }
    else if (!strcmp(cmd, COMANDO_NOVA_PARTIDA)){
        Game::novo_jogo();
        Game::lado_do_computador = VAZIO;
    }
    else if (!strcmp(cmd, COMANDO_ATIVAR_ENGINE) || !strcmp(cmd, COMANDO_FAZER_LANCE) || !strcmp(cmd, COMANDO_COMPUTADOR_CALCULAR)){
        Game::lado_do_computador = Game::lado;
    }
    else if (!strcmp(cmd, COMANDO_DESATIVAR_ENGINE)){
        Game::lado_do_computador = VAZIO;
    }
    else if (!strcmp(cmd, COMANDO_QUIT)){
        return false;
    }
    else if (!strcmp(cmd, COMANDO_ENCERRAR)){
        return false;
    }
    else if (!strcmp(cmd, COMANDO_CARREGAR_FEN)){
        char posicao[80], lado_a_jogar[1], roques[4], casa_en_passant[2], hm[4], fm[4];
		
        if (scanf("%s", posicao) == EOF){
            return false;
        }
        if (scanf("%s", lado_a_jogar) == EOF){
            return false;
        }
		if (scanf("%s", roques) == EOF){
            return false;
        }
        if (scanf("%s", casa_en_passant) == EOF){
            return false;
        }
        if (scanf("%s", hm) == EOF){
            return false;
        }
		if (scanf("%s", fm) == EOF){
            return false;
        }

		setar_posicao(posicao, lado_a_jogar, roques, casa_en_passant, hm, fm);
        return true;
    }
    else if (!strcmp(cmd, COMANDO_CONFIGURAR_PROFUNDIDADE)){
        if (scanf("%d", &profundidade_maxima) == EOF){
            return false;
        }

        tempo_maximo = 1 << 25;
        profundidade_fixa = true;
        tempo_fixo = false;
    }
    else if (!strcmp(cmd, COMANDO_CONFIGURAR_TEMPO)){
        if (scanf("%d", &tempo_maximo) == EOF){
            return false;
        }

        tempo_maximo *= SEGUNDO;
        profundidade_maxima = MAX_PLY;
        tempo_fixo = true;
        profundidade_fixa = false;
    }
    else if (!strcmp(cmd, COMANDO_TROCAR_DE_LADO)){
        Game::lado ^= 1;
        Game::xlado ^= 1;
    }
    else if (!strcmp(cmd, COMANDO_DESFAZER_LANCE)){
        if (!Game::hply){
            return true;
        }
        
        Game::lado_do_computador = VAZIO;
        desfaz_lance();
        Game::ply = 0;

        if (Game::qntt_lances_totais[0] != 0){
            Game::qntt_lances_totais[0] = 0;
        }

        Gen::gerar_lances(Game::lado, Game::xlado);
    }
    else if (!strcmp(cmd, COMANDO_INICIA_XBOARD)){
        xboard();

        return false;
    }
    else if (!strcmp(cmd, COMANDO_REALIZAR_PERFT)){
        if (scanf("%d", &profundidade_perft) == EOF){
            return false;
        }

        int tempoInicial = obter_tempo();

        unsigned long long perft_results = Gen::perft(profundidade_perft);

        int delta = obter_tempo() - tempoInicial;

        int nps_perft;

        if (delta > 0){
            nps_perft = (double) perft_results / (double) delta;
            nps_perft *= 1000.0;
        }
        else{
            nps_perft = 0;
        }

        printf("\n");
        printf("Perft: %llu\n", perft_results);
        printf("Tempo: %d ms\n", delta);
        printf("Lances por segundo: %d\n", nps_perft);
    }
    else{
        processar_lance_do_usuario(cmd);
    }

    return true;
}
