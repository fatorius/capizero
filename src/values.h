/*
Nesse arquivo estão definidos os valores usados na avaliação de 
posições da engine, o valor absoluto de casa peça, os valores dos
adicionais para peão e torre, e os valores de cada casa para as peças
*/

#ifndef VALUES
#define VALUES


// VALORES DAS PEÇAS
#define VALOR_PEAO 100
#define VALOR_CAVALO 300
#define VALOR_BISPO 315
#define VALOR_TORRE 500
#define VALOR_DAMA 900
#define VALOR_REI 10000

const int pieces_valor[6] = 
{
	VALOR_PEAO, VALOR_CAVALO, VALOR_BISPO, VALOR_TORRE, VALOR_DAMA, VALOR_REI
};


// VALORES DAS ESTRUTURAS
#define EN_PASSANT_SCORE 10
#define ISOLADO_SCORE 10

#define COLUNA_SEMI_ABERTA_BONUS 11
#define COLUNA_ABERTA_BONUS 40


// VALORES PARA ORDENAÇÃO DE LANCES
#define SCORE_ROQUE        5000000
#define SCORE_CAPTURAS_D   8000000
#define SCORE_KILLER_MOVE  9000000
#define SCORE_CONTRALANCE 10000000 
#define SCORE_CAPTURAS_V  50000000
#define PONTUACAO_HASH   100000000

// ordenação de capturas
#define SCORE_CAPTURAS_DESVANTAJOSAS SCORE_CAPTURAS_D
#define SCORE_DE_CAPTURA_VANTAJOSAS SCORE_CAPTURAS_V
#define CAPTURAS_IGUAIS 0

const int px[6] = {
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS,   //Peão x Peão
	SCORE_DE_CAPTURA_VANTAJOSAS + 20,                //Peão x Cavalo
	SCORE_DE_CAPTURA_VANTAJOSAS + 20, 		         //Peão x Bispo
	SCORE_DE_CAPTURA_VANTAJOSAS + 40,                //Peão x Torre
	SCORE_DE_CAPTURA_VANTAJOSAS + 80, 		         //Peão x Dama
	SCORE_DE_CAPTURA_VANTAJOSAS      		         //Peão x Rei
};

const int cx[6] = {
	SCORE_CAPTURAS_DESVANTAJOSAS - 20,               //Cavalo x Peao
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS,   //Cavalo x Cavalo
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS,   //Cavalo x Bispo
	SCORE_DE_CAPTURA_VANTAJOSAS + 20,                //Cavalo x Torre
	SCORE_DE_CAPTURA_VANTAJOSAS + 60, 	             //Cavalo x Dama
	SCORE_DE_CAPTURA_VANTAJOSAS                      //Cavalo x Rei
};

const int bx[6] = {
	SCORE_CAPTURAS_DESVANTAJOSAS - 20,				//Bispo x Peao  
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS,  //Bispo x Cavalo
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS, 	//Bispo x Bispo
	SCORE_DE_CAPTURA_VANTAJOSAS + 20, 				//Bispo x Torre
	SCORE_DE_CAPTURA_VANTAJOSAS + 60, 				//Bispo x Dama
	SCORE_DE_CAPTURA_VANTAJOSAS						//Bispo x Rei
};

const int tx[6] = {
	SCORE_CAPTURAS_DESVANTAJOSAS - 40,				//Torre x Peao
	SCORE_CAPTURAS_DESVANTAJOSAS - 10,				//Torre x Cavalo
	SCORE_CAPTURAS_DESVANTAJOSAS - 10,  			//Torre x Bispo
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS, 	//Torre x Torre
	SCORE_DE_CAPTURA_VANTAJOSAS + 40, 				//Torre x Dama
	SCORE_DE_CAPTURA_VANTAJOSAS						//Torre x Rei
};

const int dx[6] = {
	SCORE_CAPTURAS_DESVANTAJOSAS - 80,				//Dama x Peao
	SCORE_CAPTURAS_DESVANTAJOSAS - 60,				//Dama x Cavalo
	SCORE_CAPTURAS_DESVANTAJOSAS - 60,				//Dama x Bispo
	SCORE_CAPTURAS_DESVANTAJOSAS - 40,  			//Dama x Torre
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS, 	//Dama x Dama
	SCORE_DE_CAPTURA_VANTAJOSAS						//Dama x Rei
};

const int rx[6] = {
	SCORE_DE_CAPTURA_VANTAJOSAS + 10, 				//Rei x Peao
	SCORE_DE_CAPTURA_VANTAJOSAS + 30, 				//Rei x Cavalo
	SCORE_DE_CAPTURA_VANTAJOSAS + 30, 				//Rei x Bispo
	SCORE_DE_CAPTURA_VANTAJOSAS + 50, 				//Rei x Torre
	SCORE_DE_CAPTURA_VANTAJOSAS + 90, 				//Rei x Dama
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS	//Rei x Rei
};

// VALORES PARA PESQUISA
#define ALPHA_INICIAL -1000000
#define BETA_INICIAL 1000000

#define VALOR_XEQUE_MATE_PADRAO -999999
#define MELHOR_SCORE_INICIAL -1000001

#define VALOR_EMPATE 0
#define VALOR_XEQUE_MATE_BRANCAS 999999
#define VALOR_XEQUE_MATE_PRETAS -999999

#define VERIFICACAO_DE_LANCES 4095


// tabelas de casas
const int defesa_ala_da_dama[2][64]=
{
{
	0, 0, 0, 0, 0, 0, 0, 0,
	8,11, 8, 0, 0, 0, 0, 0,
	8, 6, 8, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
},
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	8, 6, 8, 0, 0, 0, 0, 0,
	8,11, 8, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
}};

const int defesa_ala_do_rei[2][64]=
{
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 8,11, 8,
	0, 0, 0, 0, 0, 8, 6, 8,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
},
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 8, 6, 8,
	0, 0, 0, 0, 0, 8,11, 8,
	0, 0, 0, 0, 0, 0, 0, 0
}};

const int peao_score[64] = 
{
	  0,   0,   0,   0,   0,   0,   0,   0,
      0,   2,   4, -12, -12,   4,   2,   0,
      0,   2,   4,   4,   4,   4,   2,   0,
      0,   2,   4,   8,   8,   4,   2,   0,
      0,   2,   4,   8,   8,   4,   2,   0,
      4,   8,  10,  16,  16,  10,   8,   4,
    100, 100, 100, 100, 100, 100, 100, 100,
	  0,   0,   0,   0,   0,   0,   0,   0
};

const int cavalo_score[64] = 
{
	-30, -20, -10,  -8,  -8, -10, -20, -30,
	-16, -6,   -2,   0,   0,   -2, -6, -16,
	-8,   -2,   4,   6,   6,   4,   -2, -8,
	-5,   0,   6,   8,   8,   6,   0, -5,
	-5,   0,   6,   8,   8,   6,   0, -5,
	-10,   -2,   4,   6,   6,   4,   -2, -10,
	-20, -10,   -2,   0,   0,   -2, -10, -20,
	-150, -20, -10, -5, -5, -10, -20, -150
};

const int bispo_score[64] = 
{
	-10, -10, -12, -10, -10, -12, -10, -10,
	  0,   4,   4,   4,   4,   4,   4,   0,
	  2,   4,   6,   6,   6,   6,   4,   2,
	  2,   4,   6,   8,   8,   6,   4,   2,
	  2,   4,   6,   8,   8,   6,   4,   2,
	  2,   4,   6,   6,   6,   6,   4,   2,
	-10,   4,   4,   4,   4,   4,   4, -10,
	-10, -10, -10, -10, -10, -10, -10, -10
};

const int torre_score[64] = 
{
	4, 4,  4,  6, 6,  4, 4, 4,
	0, 0,  0,  0, 0,  0, 0, 0,
	0, 0,  0,  0, 0,  0, 0, 0,
	0, 0,  0,  0, 0,  0, 0, 0,
	0, 0,  0,  0, 0,  0, 0, 0,
	0, 0,  0,  0, 0,  0, 0, 0,
	20, 20, 20, 20, 20, 20, 20, 20,
	10, 10, 10, 10, 10, 10, 10, 10
};

const int dama_score[64] = 
{
    -10, -10,  -6,  -4,  -4,  -6, -10, -10,
	-10,   2,   2,   2,   2,   2,   2, -10,
	  2,   2,   2,   3,   3,   2,   2,   2,
	  2,   2,   3,   4,   4,   3,   2,   2,
	  2,   2,   3,   4,   4,   3,   2,   2,
	  2,   2,   2,   3,   3,   2,   2,   2,
	-10,   2,   2,   2,   2,   2,   2, -10,
	-10, -10,   2,   2,   2,   2, -10, -10
};

const int rei_score[64] = 
{
     20,  20,  20, -40,  10, -60,  20,  20,     
	 15,  20, -25, -30, -30, -45,  20,  15,   
	-48, -48, -48, -48, -48, -48, -48, -48,
    -48, -48, -48, -48, -48, -48, -48, -48,
    -48, -48, -48, -48, -48, -48, -48, -48,
    -48, -48, -48, -48, -48, -48, -48, -48,
    -48, -48, -48, -48, -48, -48, -48, -48,
    -48, -48, -48, -48, -48, -48, -48, -48
};

const int rei_finais_score[64] = 
{
	  0,   8,  16,  18,  18,  16,  8,   0,
	  8,  16,  24,  32,  32,  24,  16,  8,
	 16,  24,  32,  40,  40,  32,  24,  16,
	 25,  32,  40,  48,  48,  40,  32,  25,
	 25,  32,  40,  48,  48,  40,  32,  25,
	 16,  24,  32,  40,  40,  32,  24,  16,
	 8,  16,  24,  32,  32,  24,  16,  8,
	  0,  8,  16,  18,  18,  16,  8,   0
};

const int peao_passado_score[64] = 
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	60,  60,  60,  60  ,60, 60, 60, 60,
	30, 30, 30, 30, 30, 30, 30, 30,
	15, 15, 15, 15,15, 15, 15, 15, 
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	0, 0, 0, 0, 0, 0, 0, 0
};


#endif