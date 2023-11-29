/*
Nesse arquivo estão definidos os valores usados na avaliação de 
posições da engine, o valor absoluto de casa peça, os valores dos
adicionais para peão e torre, e os valores de cada casa para as peças
*/

#ifndef VALUES
#define VALUES


// VALORES DAS PEÇAS
#define VALOR_PEAO 82
#define VALOR_CAVALO 337
#define VALOR_BISPO 365
#define VALOR_TORRE 477
#define VALOR_DAMA 1025
#define VALOR_REI 10000

// VALORES DAS PEÇAS NOS FINAIS
#define VALOR_PEAO_FINAIS 94
#define VALOR_CAVALO_FINAIS 281
#define VALOR_BISPO_FINAIS 297
#define VALOR_TORRE_FINAIS 512
#define VALOR_DAMA_FINAIS 936
#define VALOR_REI_FINAIS 10000


const int valor_material[6] = 
{
	VALOR_PEAO, VALOR_CAVALO, VALOR_BISPO, VALOR_TORRE, VALOR_DAMA, VALOR_REI
};


#define VANTAGEM_DECISIVA 9


// VALORES DAS ESTRUTURAS
#define EN_PASSANT_SCORE 10
#define ISOLADO_SCORE 7
#define PEAO_PROTEGIDO_SCORE 5
#define PEAO_PROTEGIDO_SCORE_FINAIS 9

#define COLUNA_SEMI_ABERTA_BONUS 11
#define COLUNA_ABERTA_BONUS 40

#define COLUNA_SEMI_ABERTA_BONUS_DAMA 5
#define COLUNA_ABERTA_BONUS_DAMA 20

#define REI_NO_CENTRO_MEIO_JOGO -15

// VALORES DE ATIVIDADE DAS PEÇAS
const int ATIVIDADE_CAVALO[9] = {-62,-53,-12,-4,3,13,22,28,33};
const int ATIVIDADE_BISPO[14] = {-48,-20,16,26,38,51,55,63,63,68,81,81,91,98};
const int ATIVIDADE_TORRE[15] = {-60,-20,2,3,3,11,22,31,40,40,41,48,57,57,62};  
const int ATIVIDADE_DAMA[28] = {-30,-12,-8,-9,20,23,23,35,38,53,64,65,65,66,67,67,72,72,77,79,93,108,108,108,110,114,114,116};

const int ATIVIDADE_CAVALO_FINAIS[9] = {-81,-56,-31,-16,5,11,17,20,25};
const int ATIVIDADE_BISPO_FINAIS[14] = {-59,-23,-3,13,24,42,54,57,65,73,78,86,88,97};
const int ATIVIDADE_TORRE_FINAIS[15] = {-78,-17,23,39,70,99,103,121,134,139,158,164,168,169,172};  
const int ATIVIDADE_DAMA_FINAIS[28] = {-48,-30,-7,19,40,55,59,75,78,96,96,100,121,127,131,133,136,141,147,150,151,168,168,171,182,182,192,219};



// REDUÇÕES E CONDIÇÕES
#define REDUCAO_IID /4
#define PROFUNDIDADE_CONDICAO_IID 5

// WINDOWS
#define TAMANHO_JANELA_DE_PESQUISA 20
#define READAPTACAO_JANELA_DE_PESQUISA 3

// VALORES PARA ORDENAÇÃO DE LANCES
#define SCORE_ROQUE        5000000
#define SCORE_CAPTURAS_D   8000000
#define SCORE_CONTRALANCE  9000000
#define SCORE_KILLER_2     9500000
#define SCORE_KILLER_1    10000000
#define SCORE_CAPTURAS_V  50000000
#define PONTUACAO_HASH   100000000

#define REDUCAO_LMR 3

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

const float peso_eval_meio_jogo[33] = {
	0,    0,    0,    0,    0,    0,  0.07, 0.13,
  0.2, 0.27, 0.33,  0.4, 0.47, 0.53,  0.6,  0.67, 
  0.73, 0.8, 0.87, 0.93,    1,    1,    1,     1,
     1,   1,    1,    1,    1,    1,    1,     1, 
	 1
};

const float peso_eval_final[33] = {
	0,    0,    0,    1,    1,    1,    1,  0.93,
 0.87,  0.8, 0.73, 0.67,  0.6, 0.53, 0.47,   0.4,
 0.33, 0.27,  0.2, 0.13, 0.07,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0, 
	0
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
	  0,   0,   0,   0,   0,   0,  0,   0,
    -35,  -1, -20, -23, -15,  24, 38, -22,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -14,  13,   6,  21,  23,  12, 17, -23,
     -6,   7,  26,  31,  65,  56, 25, -20,
     98, 134,  61,  95,  68, 126, 34, -11,
     0,   0,   0,   0,   0,   0,  0,   0,
};

const int peao_score_finais[64] = 
{
	  0,   0,   0,   0,   0,   0,   0,   0,
     13,   8,   8,  10,  13,   0,   2,  -7,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
     32,  24,  13,   5,  -2,   4,  17,  17,
     94, 100,  85,  67,  56,  53,  82,  84,
    178, 173, 158, 134, 147, 132, 165, 187,
      0,   0,   0,   0,   0,   0,   0,   0,
};

const int cavalo_score[64] = 
{
    -105, -21, -58, -33, -17, -28, -19,  -23,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -13,   4,  16,  13,  28,  19,  21,   -8,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -47,  60,  37,  65,  84, 129,  73,   44,
     -73, -41,  72,  36,  23,  62,   7,  -17,
    -167, -89, -34, -49,  61, -97, -15, -107,
};

const int cavalo_score_finais[64] = 
{
	-29, -51, -23, -15, -22, -18, -50, -64,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -58, -38, -13, -28, -31, -27, -63, -99,
};

const int bispo_score[64] = 
{
	 -33,  -3, -14, -21, -13, -12, -39, -21,
       4,  15,  16,   0,   7,  21,  33,   1,
       0,  15,  15,  15,  14,  27,  18,  10,
      -6,  13,  13,  26,  34,  12,  10,   4,
      -4,   5,  19,  50,  37,  37,   7,  -2,
     -16,  37,  43,  40,  35,  50,  37,  -2,
     -26,  16, -18, -13,  30,  59,  18, -47,
     -29,   4, -82, -37, -25, -42,   7,  -8,
};

const int bispo_score_finais[64] = 
{
	-23,  -9, -23,  -5, -9, -16,  -5, -17,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
     -6,   3,  13,  19,  7,  10,  -3,  -9,
     -3,   9,  12,   9, 14,  10,   3,   2,
      2,  -8,   0,  -1, -2,   6,   0,   4,
     -8,  -4,   7, -12, -3, -13,  -4, -14,
    -14, -21, -11,  -8, -7,  -9, -17, -24,
};

const int torre_score[64] = 
{
	 -19, -13,   1,  17, 16,  7, -37, -26,
     -44, -16, -20,  -9, -1, 11,  -6, -71,
     -45, -25, -16, -17,  3,  0,  -5, -33,
     -36, -26, -12,  -1,  9, -7,   6, -23,
     -24, -11,   7,  26, 24, 35,  -8, -20,
      -5,  19,  26,  36, 17, 45,  61,  16,
      27,  32,  58,  62, 80, 67,  26,  44,
      32,  42,  32,  51, 63,  9,  31,  43,
};

const int torre_score_finais[64] = 
{
	-9,  2,  3, -1, -5, -13,   4, -20,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -4,  0, -5, -1, -7, -12,  -8, -16,
     3,  5,  8,  4, -5,  -6,  -8, -11,
     4,  3, 13,  1,  2,   1,  -1,   2,
     7,  7,  7,  5,  4,  -3,  -5,  -3,
    11, 13, 13, 11, -3,   3,   8,   3,
    13, 10, 18, 15, 12,  12,   8,   5,
};

const int dama_score[64] = 
{
     -1, -18,  -9,  10, -15, -25, -31, -50,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -28,   0,  29,  12,  59,  44,  43,  45,
};

const int dama_score_finais[64] = 
{
    -33, -28, -22, -43,  -5, -32, -20, -41,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -18,  28,  19,  47,  31,  34,  39,  23,
      3,  22,  24,  45,  57,  40,  57,  36,
    -20,   6,   9,  49,  47,  35,  19,   9,
    -17,  20,  32,  41,  58,  25,  30,   0,
     -9,  22,  22,  27,  27,  19,  10,  20,
};

const int rei_score[64] = 
{
     -15,  36,  12, -54,   8, -28,  24,  14,
       1,   7,  -8, -64, -43, -16,   9,   8,
     -14, -14, -22, -46, -44, -30, -15, -27,
     -49,  -1, -27, -39, -46, -44, -33, -51,
     -17, -20, -12, -27, -30, -25, -14, -36,
      -9,  24,   2, -16, -20,   6,  22, -22,
      29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -65,  23,  16, -15, -56, -34,   2,  13,
};

const int rei_finais_score[64] = 
{
	  -53, -34, -21, -11, -28, -14, -24, -43
      -27, -11,   4,  13,  14,   4,  -5, -17,
      -19,  -3,  11,  21,  23,  16,   7,  -9,
      -18,  -4,  21,  24,  27,  23,   9, -11,
       -8,  22,  24,  27,  26,  33,  26,   3,
       10,  17,  23,  15,  20,  45,  44,  13,
      -12,  17,  14,  17,  17,  38,  23,  11,
      -74, -35, -18, -18, -11,  15,   4, -17,
};

const int peao_passado_score[64] = 
{
	  0, 0, 0, 0, 0, 0, 0, 0,
	 40,  40,  40,  40,  40,  40,  40,  40,
	 40,  40,  40,  40,  40,  40,  40,  40,
	 40,  40,  40,  40,  40,  40,  40,  40,
	 50,  50,  50,  50,  50,  50,  50,  50,
	 80,  80,  80,  80,  80,  80,  80,  80,
	130, 130, 130, 130, 130, 130, 130, 130,
	0, 0, 0, 0, 0, 0, 0, 0
};

#endif