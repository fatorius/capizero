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

int nps;
int lance_usuario;
int tabuleiro_invertido = 0;

int Interface::no_lances = 0;
int Interface::lookup;

extern int Interface::lance_inicio, Interface::lance_destino;

int Interface::tempo_gasto;

int Interface::tempo_maximo, Interface::profundidade_maxima;
bool Interface::tempo_fixo, Interface::profundidade_fixa;

int profundidade_perft;

void Interface::exibir_melhor_linha(int profundidade){
     Interface::lance_inicio = Hash::hash_inicio;
     Interface::lance_destino = Hash::hash_destino;

     for (int x = 0; x < profundidade; x++){
        if (Hash::hash_lookup(Game::lado) == false){
            break;
        }

        printf(" ");
        print_lance_algebrico(Hash::hash_inicio, Hash::hash_destino);
        Update::fazer_lance(Hash::hash_inicio, Hash::hash_destino);
     }

     while (Game::ply){
        Update::desfaz_lance();
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

void Interface::print_lance_algebrico(int a, int b){
    converter_casa_para_algebrico(a);
    converter_casa_para_algebrico(b);
}

int Interface::obter_tempo(){
    return chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
}

void Interface::display_tabuleiro() {
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
        if (Game::lista_do_jogo[i].hash == Hash::chaveAtual && Game::lista_do_jogo[i].lock == Hash::lockAtual){
            r++;
        }
    }

    return r;
}

void Interface::print_resultado(){
    int i;
    bool existem_lances_legais = false;

    Eval::atualizar_materiais();
    Gen::gerar_lances(Game::lado, Game::xlado);

    for (i = 0; i < Game::qntt_lances_totais[1]; ++i){
        if (Update::fazer_lance(Gen::lista_de_lances[i].inicio, Gen::lista_de_lances[i].destino)){
            Update::desfaz_lance();
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

void Interface::lance_computador(bool verbose){
    Game::jogador[Game::lado] = 1;

    Search::pensar(verbose);

    Interface::no_lances++;

    Hash::chaveAtual = Hash::obter_chave();
    Hash::lockAtual = Hash::obter_lock();

    Interface::lookup = Hash::hash_lookup(Game::lado);

    Interface::tempo_gasto = Interface::obter_tempo() - Search::tempo_do_inicio;

    if (verbose){
        if (Interface::lance_inicio != 0 || Interface::lance_destino != 0){
            Hash::hash_inicio = Interface::lance_inicio;
            Hash::hash_destino = Interface::lance_destino;
        }
        else{
            printf("(Sem lances legais) \n");
            Game::lado_do_computador = VAZIO;

            display_tabuleiro();

            Gen::gerar_lances(Game::lado, Game::xlado);

            return;
        }
    }
    
    Update::fazer_lance(Hash::hash_inicio, Hash::hash_destino);

    Eval::atualizar_materiais();

    Game::ply = 0;
    Game::qntt_lances_totais[0] = 0;

    Gen::gerar_lances(Game::lado, Game::xlado);

    if (verbose){
        printf("Lance do computador: %s \n", lance_para_string(Hash::hash_inicio, Hash::hash_destino, 0));
        printf("\n");

        printf("\nTempo gasto: %d ms \n", Interface::tempo_gasto);

        if (Interface::tempo_gasto > 0){
            nps = (double) Search::lances_avaliados / (double) Interface::tempo_gasto;
            nps *= 1000.0;
        }
        else{
            nps = 0;
        }

        printf("\nLances por segundo: %d \n", (int) nps);

        print_resultado();

        printf("Lance ");
        printf("%d", Interface::no_lances++);

        display_tabuleiro();
    }
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

    lance_usuario = Interface::converter_lance(lnc);

    if (lance_usuario == -1 || !Update::fazer_lance(Gen::lista_de_lances[lance_usuario].inicio, Gen::lista_de_lances[lance_usuario].destino)){
        printf("Comando / Lance inválido\n");
        printf("Digite 'ajuda' para exibir uma lista de comandos válidos ou\n");
        printf("Digite 'lances' para exibir uma lista de lances legais\n");
        printf("\n");
        return;
    }

    // atualiza peça promovida
    if (Gen::lista_de_lances[Game::hply - 1].promove > P && (Consts::colunas[Gen::lista_de_lances[lance_usuario].destino] == FILEIRA_1 || Consts::colunas[Gen::lista_de_lances[lance_usuario].destino] == FILEIRA_8)){
        Update::remover_piece(Game::xlado, D, Gen::lista_de_lances[lance_usuario].destino);

        if (lnc[PROMOCAO] == 'n' || lnc[PROMOCAO] == 'N'){
            Update::adicionar_piece(Game::xlado, C, Gen::lista_de_lances[lance_usuario].destino);
        }
        else if (lnc[PROMOCAO] == 'b' || lnc[PROMOCAO] == 'B'){
            Update::adicionar_piece(Game::xlado, B, Gen::lista_de_lances[lance_usuario].destino);
        }
        else if (lnc[PROMOCAO] == 'r' || lnc[PROMOCAO] == 'R'){
            Update::adicionar_piece(Game::xlado, T, Gen::lista_de_lances[lance_usuario].destino);
        }
        else{
            Update::adicionar_piece(Game::xlado, D, Gen::lista_de_lances[lance_usuario].destino);
        }
    }

    return;
}

bool Interface::ler_comando(){
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
        Help::exibir_ajuda();
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

		Update::setar_posicao(posicao, lado_a_jogar, roques, casa_en_passant, hm, fm);
        return true;
    }
    else if (!strcmp(cmd, COMANDO_CONFIGURAR_PROFUNDIDADE)){
        if (scanf("%d", &Interface::profundidade_maxima) == EOF){
            return false;
        }

        Interface::tempo_maximo = 1 << 25;
        Interface::profundidade_fixa = true;
        Interface::tempo_fixo = false;
    }
    else if (!strcmp(cmd, COMANDO_CONFIGURAR_TEMPO)){
        if (scanf("%d", &Interface::tempo_maximo) == EOF){
            return false;
        }

        Interface::tempo_maximo *= SEGUNDO;
        Interface::profundidade_maxima = MAX_PLY;
        Interface::tempo_fixo = true;
        Interface::profundidade_fixa = false;
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
        Update::desfaz_lance();
        Game::ply = 0;

        if (Game::qntt_lances_totais[0] != 0){
            Game::qntt_lances_totais[0] = 0;
        }

        Gen::gerar_lances(Game::lado, Game::xlado);
    }
    else if (!strcmp(cmd, COMANDO_INICIA_XBOARD)){
        Xboard::xboard();

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
