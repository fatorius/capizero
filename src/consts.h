#ifndef CONSTS
#define CONSTS

#define COLUNA_A 0
#define COLUNA_B 1
#define COLUNA_C 2
#define COLUNA_D 3
#define COLUNA_E 4
#define COLUNA_F 5
#define COLUNA_G 6
#define COLUNA_H 7

#define A1	0
#define B1	1
#define C1	2
#define D1	3
#define E1	4
#define F1  5
#define G1	6
#define H1	7

#define A2	8
#define B2	9
#define C2	10
#define D2	11
#define E2	12
#define F2	13
#define G2	14
#define H2	15

#define A3	16
#define B3	17
#define C3	18
#define D3	19
#define E3	20
#define F3	21
#define G3	22
#define H3	23

#define A4	24
#define B4	25
#define C4	26
#define D4	27
#define E4	28
#define F4	29
#define G4	30
#define H4	31

#define A5	32
#define B5	33
#define C5	34
#define D5	35
#define E5	36
#define F5	37
#define G5	38
#define H5	39

#define A6	40
#define B6	41
#define C6	42
#define D6	43
#define E6	44
#define F6	45
#define G6	46
#define H6	47

#define A7	48
#define B7	49
#define C7	50
#define D7	51
#define E7	52
#define F7	53
#define G7	54
#define H7	55

#define A8	56
#define B8	57
#define C8	58
#define D8	59
#define E8	60
#define F8	61
#define G8	62
#define H8	63

#define NORTE 0
#define NE 1
#define LESTE 2
#define SE 3
#define SUL 4
#define SO 5
#define OESTE 6
#define NO 7

#define AVANCO_DUPLO 16

#define P 0
#define C 1
#define B 2
#define T 3
#define D 4
#define R 5
#define VAZIO 6

#define BRANCAS 0
#define PRETAS 1

#define CASAS_DO_TABULEIRO 64
#define LADOS 2
#define TIPOS_DE_PIECES 6

#define MAX_PLY 64

#define PILHA_DE_LANCES 4000
#define PILHA_DO_JOGO 2000

#define SCORE_DE_CAPTURA 10000000

#define BRANCAS_ROQUE_MENOR 1
#define BRANCAS_ROQUE_MAIOR 2
#define PRETAS_ROQUE_MENOR 4
#define PRETAS_ROQUE_MAIOR 8

const char piece_char[6] = 
{
	'P', 'C', 'B', 'T', 'D', 'R'
};

#define VALOR_PEAO 100
#define VALOR_CAVALO 300
#define VALOR_BISPO 300
#define VALOR_TORRE 500
#define VALOR_DAMA 900
#define VALOR_REI 10000

#define EN_PASSANT_SCORE 10

#define PROMOCAO 4

#define FILEIRA_8 7
#define FILEIRA_7 6
#define FILEIRA_6 5
#define FILEIRA_5 4
#define FILEIRA_4 3
#define FILEIRA_3 2
#define FILEIRA_2 1
#define FILEIRA_1 0

const int px[6] = {0,10,20,30,40,0};
const  int cx[6] = {-3,7,17,27,37,0};
const int bx[6] = {-3,7,17,27,37,0};
const int tx[6] = {-5,5,15,25,35,0};
const int dx[6] = {-9,1,11,21,31,0};
const int rx[6] = {0,10,20,30,40,0};

const int pieces_valor[6] = 
{
	VALOR_PEAO, VALOR_CAVALO, VALOR_BISPO, VALOR_TORRE, VALOR_DAMA, VALOR_REI
};

const int cor_inicial[64] = 
{
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1
};

const int tabuleiro_inicial[64] = 
{
	3, 1, 2, 4, 5, 2, 1, 3,
	0, 0, 0, 0, 0, 0, 0, 0,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	0, 0, 0, 0, 0, 0, 0, 0,
	3, 1, 2, 4, 5, 2, 1, 3
};

const int colunas[64]=
{
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7,
	0, 1, 2, 3, 4, 5, 6, 7
};

const int linhas[64]=
{
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7
};

const int no_diag[64]={
	 14,13,12,11,10, 9, 8, 7,
	 13,12,11,10, 9, 8, 7, 6,
	 12,11,10, 9, 8, 7, 6, 5,
	 11,10, 9, 8, 7, 6, 5, 4,
	 10, 9, 8, 7, 6, 5, 4, 3,
	  9, 8, 7, 6, 5, 4, 3, 2,
	  8, 7, 6, 5, 4, 3, 2, 1,
	  7, 6, 5, 4, 3, 2, 1, 0
};

const int ne_diag[64]={
	 7, 8,9,10,11,12,13,14,
	 6, 7,8, 9,10,11,12,13,
	 5, 6,7, 8, 9,10,11,12,
	 4, 5,6, 7, 8, 9,10,11,
	 3, 4,5, 6, 7, 8, 9,10,
	 2, 3,4, 5, 6, 7, 8, 9,
	 1, 2,3, 4, 5, 6, 7, 8,
	 0, 1,2, 3, 4, 5, 6, 7
};

const int flip[64] = 
{
	 56,  57,  58,  59,  60,  61,  62,  63,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 16,  17,  18,  19,  20,  21,  22,  23,
	  8,   9,  10,  11,  12,  13,  14,  15,
	  0,   1,   2,   3,   4,   5,   6,   7
};

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

const int roque_mask[64] = {
	13, 15, 15, 15, 12, 15, 15, 14,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15,
	7, 15, 15, 15,  3, 15, 15, 11
};

const int lsb_64_table[64] =
{
   63, 30,  3, 32, 59, 14, 11, 33,
   60, 24, 50,  9, 55, 19, 21, 34,
   61, 29,  2, 53, 51, 23, 41, 18,
   56, 28,  1, 43, 46, 27,  0, 35,
   62, 31, 58,  4,  5, 49, 54,  6,
   15, 52, 12, 40,  7, 42, 45, 16,
   25, 57, 48, 13, 10, 39,  8, 44,
   20, 47, 38, 22, 17, 37, 36, 26
};

#endif