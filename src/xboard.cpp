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
	int prom;

	signal(SIGINT, SIG_IGN);
	printf("\n");
	novo_jogo();

	tempo_fixo = 0;
	lado_do_computador = VAZIO;
    
	while(true){

		fflush(stdout);

		if (lado == lado_do_computador){
			prom = 0;
			pensar(true);
			atualizar_materiais();
			gerar_lances(lado, xlado);
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
            
			if ((tabuleiro[hash_inicio] == P) && (linhas[hash_destino] == fileira_de_promocao[lado])){
				prom = D; // ASSUME QUE O JOGADOR IRÁ PROMOVER SEMPRE PARA DAMA
			}

			printf("move %s\n", lance_para_string(hash_inicio,hash_destino,prom));
	
			fazer_lance(hash_inicio,hash_destino);
  
			ply = 0;
			gerar_lances(lado, xlado);
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
			novo_jogo();
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

		if (!strcmp(comando, "white")) {
			lado = BRANCAS;
			xlado = PRETAS;
			gerar_lances(lado, xlado);
			lado_do_computador = PRETAS;
			continue;
		}

		if (!strcmp(comando, "black")) 
		{
			lado = PRETAS;
			xlado = BRANCAS;
			gerar_lances(lado, xlado);
			lado_do_computador = BRANCAS;
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
			lado_do_computador = lado;
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
			if (!hply){
				continue;
            }

			desfaz_lance();
			ply = 0;

			gerar_lances(lado, xlado);
			
            continue;
		}

		if (!strcmp(comando, "remove")) {
			if (hply < 2){
				continue;
            }
			
            desfaz_lance();
			desfaz_lance();

			ply = 0;

			gerar_lances(lado, xlado);
			continue;
		}
		if (!strcmp(comando, "post")){
			continue;
		}

		if (!strcmp(comando, "nopost")) {
			continue;
		}

		qntt_lances_totais[0] = 0;
		gerar_lances(lado, xlado);

		m = converter_lance(linha);
		if (m == -1 || !fazer_lance(lista_de_lances[m].inicio, lista_de_lances[m].destino)){
			printf("Error (unknown comand): %s\n", comando);
        }
		else {
			ply = 0;
 			gerar_lances(lado, xlado);
			print_resultado();
		}
	}
}
