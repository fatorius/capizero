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

void xboard(){
	int lado_do_computador;
	char linha[256], comando[256];
	int m;
	int n;
	int prom;

	signal(SIGINT, SIG_IGN);
	printf("\n");
	Game::novo_jogo();

	tempo_fixo = 0;
	lado_do_computador = VAZIO;
    
	while(true){

		fflush(stdout);

		if (Game::lado == lado_do_computador){
			prom = 0;
			pensar(true);
			Eval::atualizar_materiais();
			gerar_lances(Game::lado, Game::xlado);
			chaveAtual = obter_chave();
			lockAtual = obter_lock();

			if(lance_inicio != 0 || lance_destino != 0){
				hash_inicio = lance_inicio;
				hash_destino = lance_destino;
			}
			else{
				printf(" lookup=0 ");
            }

			lista_de_lances[0].inicio = hash_inicio;
            lista_de_lances[0].destino = hash_destino;
            
			if ((Bitboard::tabuleiro[hash_inicio] == P) && (Consts::linhas[hash_destino] == fileira_de_promocao[Game::lado])){
				prom = D; // ASSUME QUE O JOGADOR IRÁ PROMOVER SEMPRE PARA DAMA
			}

			printf("move %s\n", lance_para_string(hash_inicio,hash_destino,prom));
	
			fazer_lance(hash_inicio,hash_destino);
  
			Game::ply = 0;
			gerar_lances(Game::lado, Game::xlado);
			print_resultado();
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
			gerar_lances(Game::lado, Game::xlado);
			lado_do_computador = PRETAS;
			continue;
		}

		if (!strcmp(comando, "black")){
			Game::lado = PRETAS;
			Game::xlado = BRANCAS;
			gerar_lances(Game::lado, Game::xlado);
			lado_do_computador = BRANCAS;
			continue;
		}

		if (!strcmp(comando, "d")){
			display_tabuleiro();
			continue;
		}
		
        if (!strcmp(comando, "st")){
			sscanf(linha, "st %d", &tempo_maximo);
			tempo_maximo *= 1000;
			profundidade_maxima = MAX_PLY;
			tempo_fixo = true;
			continue;
		}

		if (!strcmp(comando, "sd")) {
			sscanf(linha, "sd %d", &profundidade_maxima);
			tempo_maximo = 1 << 25;
			continue;
		}

		if (!strcmp(comando, "time")) {
			sscanf(linha, "time %d", &tempo_maximo);
			if(tempo_maximo < 200){
			    profundidade_maxima = 1;
            }
			else{
				tempo_maximo /= 2;
				profundidade_maxima = MAX_PLY;
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
			pensar(true);
			chaveAtual = obter_chave();
			lockAtual = obter_lock();
            
			if(hash_inicio==0 && hash_destino==0){
				continue;
            }
			
            printf("Hint: %s\n", lance_para_string(hash_inicio,hash_destino,0));
			
            continue;
		}
		
        if (!strcmp(comando, "undo")){
			if (!Game::hply){
				continue;
            }

			desfaz_lance();
			Game::ply = 0;

			gerar_lances(Game::lado, Game::xlado);
			
            continue;
		}

		if (!strcmp(comando, "remove")) {
			if (Game::hply < 2){
				continue;
            }
			
            desfaz_lance();
			desfaz_lance();

			Game::ply = 0;

			gerar_lances(Game::lado, Game::xlado);
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
			setar_posicao(posicao, lado_a_jogar, roques, casa_en_passant, hm, fm);
			continue;
		}

		if (!strcmp(comando, "protover")) {
			sscanf(linha, "protover %d", &n);
			printf("feature ping=1 analyze=0 setboard=1 myname=capizero_%s name=0 nps=0\n", CAPIZERO_VERSION);
			continue;
		}

		Game::qntt_lances_totais[0] = 0;
		gerar_lances(Game::lado, Game::xlado);

		m = converter_lance(linha);
		if (m == -1 || !fazer_lance(lista_de_lances[m].inicio, lista_de_lances[m].destino)){
			printf("Error (unknown comand): %s\n", comando);
        }
		else {
			Game::ply = 0;
 			gerar_lances(Game::lado, Game::xlado);
			print_resultado();
		}
	}
}
