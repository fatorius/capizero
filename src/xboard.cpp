#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "xboard.h"
#include "interface.h"
#include "game.h"
#include "search.h"
#include "gen.h"
#include "eval.h"
#include "hash.h"
#include "update.h"

void Xboard::xboard(){
	int lado_do_computador;
	char linha[256], comando[256];
	int m;
	int n;
	int prom;

	signal(SIGINT, SIG_IGN);
	printf("\n");
	Game::novo_jogo();

	Interface::tempo_fixo = 0;
	lado_do_computador = VAZIO;
    
	while(true){

		fflush(stdout);

		if (Game::lado == lado_do_computador){
			prom = 0;
			Search::pensar(true);
			Eval::atualizar_materiais();
			Gen::gerar_lances(Game::lado, Game::xlado);
			Hash::chaveAtual = Hash::obter_chave();
			Hash::lockAtual = Hash::obter_lock();

			if(Interface::lance_inicio != 0 || Interface::lance_destino != 0){
				Hash::hash_inicio = Interface::lance_inicio;
				Hash::hash_destino = Interface::lance_destino;
			}
			else{
				printf(" lookup=0 ");
            }

			Gen::lista_de_lances[0].inicio = Hash::hash_inicio;
            Gen::lista_de_lances[0].destino = Hash::hash_destino;
            
			if ((Bitboard::tabuleiro[Hash::hash_inicio] == P) && (Consts::linhas[Hash::hash_destino] == Update::fileira_de_promocao[Game::lado])){
				prom = D; // ASSUME QUE O JOGADOR IRÁ PROMOVER SEMPRE PARA DAMA
			}

			printf("move %s\n", Interface::lance_para_string(Hash::hash_inicio,Hash::hash_destino,prom));
	
			Update::fazer_lance(Hash::hash_inicio,Hash::hash_destino);
  
			Game::ply = 0;
			Gen::gerar_lances(Game::lado, Game::xlado);
			Interface::print_resultado();
			continue;
		}

		if (!fgets(linha, 256, stdin)){
			return;
        }

		if (linha[0] == '\n'){
			continue;
        }

		sscanf(linha, "%s", comando);
		
        if (!strcmp(comando, "xboard")){
			continue;
        }
		
        if (!strcmp(comando, "new")){
			Game::novo_jogo();
			lado_do_computador = PRETAS;
			continue;
		}
		
        if (!strcmp(comando, "quit")){
			return;
        }
		
        if (!strcmp(comando, "force")){
			lado_do_computador = VAZIO;
			continue;
		}

		if (!strcmp(comando, "white")){
			Game::lado = BRANCAS;
			Game::xlado = PRETAS;
			Gen::gerar_lances(Game::lado, Game::xlado);
			lado_do_computador = PRETAS;
			continue;
		}

		if (!strcmp(comando, "black")){
			Game::lado = PRETAS;
			Game::xlado = BRANCAS;
			Gen::gerar_lances(Game::lado, Game::xlado);
			lado_do_computador = BRANCAS;
			continue;
		}

		if (!strcmp(comando, "d")){
			Interface::display_tabuleiro();
			continue;
		}
		
        if (!strcmp(comando, "st")){
			sscanf(linha, "st %d", &Interface::tempo_maximo);
			Interface::tempo_maximo *= 1000;
			Interface::profundidade_maxima = MAX_PLY;
			Interface::tempo_fixo = true;
			continue;
		}

		if (!strcmp(comando, "sd")) {
			sscanf(linha, "sd %d", &Interface::profundidade_maxima);
			Interface::tempo_maximo = 1 << 25;
			continue;
		}

		if (!strcmp(comando, "time")) {
			sscanf(linha, "time %d", &Interface::tempo_maximo);
			if(Interface::tempo_maximo < 200){
			    Interface::profundidade_maxima = 1;
            }
			else{
				Interface::tempo_maximo /= 2;
				Interface::profundidade_maxima = MAX_PLY;
			}
			continue;
		}

		if (!strcmp(comando, "otim")) {
			continue;
		}

		if (!strcmp(comando, "go")) {
			lado_do_computador = Game::lado;
			continue;
		}

		if (!strcmp(comando, "random")){ 
			continue;
        }

		if (!strcmp(comando, "level")){ 
			continue;
        }

		if (!strcmp(comando, "hard")){ 
			continue;
        }

		if (!strcmp(comando, "easy")){ 
			continue;
        }

		if (!strcmp(comando, "hint")) {
			Search::pensar(true);
			Hash::chaveAtual = Hash::obter_chave();
			Hash::lockAtual = Hash::obter_lock();
            
			if(Hash::hash_inicio==0 && Hash::hash_destino==0){
				continue;
            }
			
            printf("Hint: %s\n", Interface::lance_para_string(Hash::hash_inicio, Hash::hash_destino, 0));
			
            continue;
		}
		
        if (!strcmp(comando, "undo")){
			if (!Game::hply){
				continue;
            }

			Update::desfaz_lance();
			Game::ply = 0;

			Gen::gerar_lances(Game::lado, Game::xlado);
			
            continue;
		}

		if (!strcmp(comando, "remove")) {
			if (Game::hply < 2){
				continue;
            }
			
            Update::desfaz_lance();
			Update::desfaz_lance();

			Game::ply = 0;

			Gen::gerar_lances(Game::lado, Game::xlado);
			continue;
		}
		if (!strcmp(comando, "post")){
			continue;
		}

		if (!strcmp(comando, "nopost")) {
			continue;
		}

		if (!strcmp(comando, "ping")){
			sscanf(linha, "ping %d", &n);
			printf("pong %d\n", n);
			continue;
		}

		if (!strcmp(comando, "setboard")){
			char posicao[80], lado_a_jogar[1], roques[4], casa_en_passant[2], hm[4], fm[4];
			sscanf(linha, "setboard %s %s %s %s %s %s", posicao, lado_a_jogar, roques, casa_en_passant, hm, fm);
			Update::setar_posicao(posicao, lado_a_jogar, roques, casa_en_passant, hm, fm);
			continue;
		}

		if (!strcmp(comando, "protover")) {
			sscanf(linha, "protover %d", &n);
			printf("feature ping=1 analyze=0 setboard=1 myname=capizero_%s name=0 nps=0\n", CAPIZERO_VERSION);
			continue;
		}

		Game::qntt_lances_totais[0] = 0;
		Gen::gerar_lances(Game::lado, Game::xlado);

		m = Interface::converter_lance(linha);
		if (m == -1 || !Update::fazer_lance(Gen::lista_de_lances[m].inicio, Gen::lista_de_lances[m].destino)){
			printf("Error (unknown comand): %s\n", comando);
        }
		else {
			Game::ply = 0;
 			Gen::gerar_lances(Game::lado, Game::xlado);
			Interface::print_resultado();
		}
	}
}
