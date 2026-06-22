/*
Nesse arquivo estão definidos os valores usados na avaliação de
posições da engine, o valor absoluto de casa peça, os valores dos
adicionais para peão e torre, e os valores de cada casa para as peças
*/

#ifndef VALUES
#define VALUES

namespace Values
{
// VALORES DAS PEÇAS
#define VALOR_PEAO 100
#define VALOR_CAVALO 300
#define VALOR_BISPO 315
#define VALOR_TORRE 500
#define VALOR_DAMA 900
#define VALOR_REI 10000

const int pieces_valor[6] = {VALOR_PEAO, VALOR_CAVALO, VALOR_BISPO, VALOR_TORRE, VALOR_DAMA, VALOR_REI};

// VALORES DAS ESTRUTURAS
#define EN_PASSANT_SCORE 10
#define ISOLADO_SCORE 10

#define COLUNA_SEMI_ABERTA_BONUS 11
#define COLUNA_ABERTA_BONUS 40

#define BISHOP_PAIR_MG 15
#define BISHOP_PAIR_EG 45

// King safety
#define KS_WEIGHT_C 22
#define KS_WEIGHT_B -15
#define KS_WEIGHT_T 89
#define KS_WEIGHT_D 144
#define KS_SCALE 256

#define PHASE_PEAO 0
#define PHASE_CAVALO 1
#define PHASE_BISPO 1
#define PHASE_TORRE 2
#define PHASE_DAMA 4
#define PHASE_REI 0
#define PHASE_MAX 24

const int mobilidade_cavalo_mg[9] = {
	-82, -39, -29, -30, -29, -31, -34, -32,
	-32};

const int mobilidade_cavalo_eg[9] = {
	-25, 24, 6, 3, 19, 31, 28, 36,
	29};

const int mobilidade_bispo_mg[14] = {
	-71, -56, -43, -41, -34, -26, -19, -21,
	-19, -20, -12, -5, 28, 42};

const int mobilidade_bispo_eg[14] = {
	-59, -51, -25, 1, 8, 24, 24, 37,
	39, 50, 39, 51, 26, 21};

const int mobilidade_torre_mg[15] = {
	-41, -24, -31, -22, -15, -6, -2, 0,
	-1, 7, 9, 16, 24, -8, 1};

const int mobilidade_torre_eg[15] = {
	-45, -28, -28, -12, -11, 1, 0, -2,
	7, -1, 1, -7, -10, -8, -13};

const int mobilidade_dama_mg[28] = {
	-21, -23, -22, -18, -17, -11, -14, -13,
	-7, -7, -4, -5, -1, 7, 0, -4,
	3, 7, 4, 17, 19, 54, -21, 8,
	-44, -7, 11, -72};

const int mobilidade_dama_eg[28] = {
	-18, 1, -17, -19, -20, -9, -15, 0,
	-15, -1, 4, 19, 22, 14, 12, 24,
	11, 10, 2, -16, -26, -105, -31, -64,
	-51, -89, -23, -152};

// REDUÇÕES E CONDIÇÕES
#define REDUCAO_IID / 4
#define PROFUNDIDADE_CONDICAO_IID 5

#define TAMANHO_JANELA_DE_PESQUISA 20
#define MAX_ASPIRATION_FAILS 3

// VALORES PARA ORDENAÇÃO DE LANCES
#define SCORE_ROQUE 5000000
#define SCORE_CAPTURAS_D 8000000
#define SCORE_CONTRALANCE 9000000
#define SCORE_KILLER_2 9500000
#define SCORE_KILLER_1 10000000
#define SCORE_CAPTURAS_V 50000000
#define PONTUACAO_HASH 100000000

#define SCORE_PROMO_Q_CAP (SCORE_CAPTURAS_V + 1000000)
#define SCORE_PROMO_Q 60000000
#define SCORE_PROMO_N_CAP 18000000
#define SCORE_PROMO_N 17000000

#define REDUCAO_LMR 3

#define R_NULL_LOW 2
#define R_NULL_HIGH 3
#define R_NULL_DEPTH_THRESH 6

#define FUTILITY_DEPTH_THRESH 6
#define FUTILITY_MARGIN_PER_PLY 100 // 1 pawn per ply
#define FUTILITY_MARGIN_FP_EXTRA 50 // FP gets a little extra slack vs RFP

// ordenação de capturas
#define SCORE_CAPTURAS_DESVANTAJOSAS SCORE_CAPTURAS_D
#define SCORE_DE_CAPTURA_VANTAJOSAS SCORE_CAPTURAS_V
#define CAPTURAS_IGUAIS 0

const int px[6] = {
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS, // Peão x Peão
	SCORE_DE_CAPTURA_VANTAJOSAS + 20,			   // Peão x Cavalo
	SCORE_DE_CAPTURA_VANTAJOSAS + 20,			   // Peão x Bispo
	SCORE_DE_CAPTURA_VANTAJOSAS + 40,			   // Peão x Torre
	SCORE_DE_CAPTURA_VANTAJOSAS + 80,			   // Peão x Dama
	SCORE_DE_CAPTURA_VANTAJOSAS					   // Peão x Rei
};

const int cx[6] = {
	SCORE_CAPTURAS_DESVANTAJOSAS - 20,			   // Cavalo x Peao
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS, // Cavalo x Cavalo
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS, // Cavalo x Bispo
	SCORE_DE_CAPTURA_VANTAJOSAS + 20,			   // Cavalo x Torre
	SCORE_DE_CAPTURA_VANTAJOSAS + 60,			   // Cavalo x Dama
	SCORE_DE_CAPTURA_VANTAJOSAS					   // Cavalo x Rei
};

const int bx[6] = {
	SCORE_CAPTURAS_DESVANTAJOSAS - 20,			   // Bispo x Peao
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS, // Bispo x Cavalo
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS, // Bispo x Bispo
	SCORE_DE_CAPTURA_VANTAJOSAS + 20,			   // Bispo x Torre
	SCORE_DE_CAPTURA_VANTAJOSAS + 60,			   // Bispo x Dama
	SCORE_DE_CAPTURA_VANTAJOSAS					   // Bispo x Rei
};

const int tx[6] = {
	SCORE_CAPTURAS_DESVANTAJOSAS - 40,			   // Torre x Peao
	SCORE_CAPTURAS_DESVANTAJOSAS - 10,			   // Torre x Cavalo
	SCORE_CAPTURAS_DESVANTAJOSAS - 10,			   // Torre x Bispo
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS, // Torre x Torre
	SCORE_DE_CAPTURA_VANTAJOSAS + 40,			   // Torre x Dama
	SCORE_DE_CAPTURA_VANTAJOSAS					   // Torre x Rei
};

const int dx[6] = {
	SCORE_CAPTURAS_DESVANTAJOSAS - 80,			   // Dama x Peao
	SCORE_CAPTURAS_DESVANTAJOSAS - 60,			   // Dama x Cavalo
	SCORE_CAPTURAS_DESVANTAJOSAS - 60,			   // Dama x Bispo
	SCORE_CAPTURAS_DESVANTAJOSAS - 40,			   // Dama x Torre
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS, // Dama x Dama
	SCORE_DE_CAPTURA_VANTAJOSAS					   // Dama x Rei
};

const int rx[6] = {
	SCORE_DE_CAPTURA_VANTAJOSAS + 10,			  // Rei x Peao
	SCORE_DE_CAPTURA_VANTAJOSAS + 30,			  // Rei x Cavalo
	SCORE_DE_CAPTURA_VANTAJOSAS + 30,			  // Rei x Bispo
	SCORE_DE_CAPTURA_VANTAJOSAS + 50,			  // Rei x Torre
	SCORE_DE_CAPTURA_VANTAJOSAS + 90,			  // Rei x Dama
	SCORE_DE_CAPTURA_VANTAJOSAS + CAPTURAS_IGUAIS // Rei x Rei
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
const int defesa_ala_da_dama[2][64] = {
	{0, 0, 0, 0, 0, 0, 0, 0,
	 8, 11, 8, 0, 0, 0, 0, 0,
	 8, 6, 8, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 8, 6, 8, 0, 0, 0, 0, 0,
	 8, 11, 8, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0}
};

const int defesa_ala_do_rei[2][64] = {
	{0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 8, 11, 8,
	 0, 0, 0, 0, 0, 8, 6, 8,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 8, 6, 8,
	 0, 0, 0, 0, 0, 8, 11, 8,
	 0, 0, 0, 0, 0, 0, 0, 0}
};

const int peao_score_mg[64] = {
	82, 82, 82, 82, 82, 82, 82, 82,
	75, 99, 74, 78, 76, 112, 148, 73,
	90, 94, 90, 96, 95, 93, 130, 86,
	75, 91, 89, 117, 115, 108, 105, 61,
	84, 105, 94, 118, 123, 105, 106, 66,
	95, 82, 92, 142, 128, 127, 123, 87,
	286, 251, 229, 255, 234, 175, 124, 142,
	82, 82, 82, 82, 82, 82, 82, 82};

const int peao_score_eg[64] = {
	94, 94, 94, 94, 94, 94, 94, 94,
	152, 144, 138, 125, 138, 136, 120, 118,
	138, 133, 116, 110, 117, 122, 108, 120,
	156, 148, 128, 119, 114, 108, 129, 134,
	177, 160, 141, 128, 124, 129, 149, 157,
	250, 247, 231, 205, 190, 185, 208, 227,
	355, 352, 318, 301, 310, 322, 352, 346,
	94, 94, 94, 94, 94, 94, 94, 94};

const int cavalo_score_mg[64] = {
	232, 290, 248, 281, 293, 302, 287, 297,
	267, 272, 310, 332, 324, 340, 310, 277,
	283, 326, 329, 346, 368, 344, 343, 296,
	299, 313, 346, 340, 350, 353, 361, 316,
	290, 340, 371, 368, 365, 396, 353, 355,
	294, 372, 284, 330, 372, 357, 388, 387,
	324, 306, 396, 175, 200, 406, 314, 339,
	181, 239, 190, 165, 333, 125, 399, 300};

const int cavalo_score_eg[64] = {
	251, 211, 235, 268, 256, 248, 208, 188,
	202, 270, 232, 274, 283, 255, 228, 221,
	262, 270, 275, 310, 298, 286, 271, 245,
	259, 295, 321, 327, 315, 326, 295, 260,
	305, 295, 343, 334, 339, 318, 308, 269,
	266, 280, 368, 350, 326, 338, 273, 210,
	243, 310, 269, 363, 368, 266, 279, 235,
	306, 323, 299, 277, 312, 278, 310, 249};

const int bispo_score_mg[64] = {
	293, 355, 350, 333, 351, 335, 291, 353,
	368, 373, 373, 354, 359, 388, 384, 353,
	366, 373, 356, 366, 364, 387, 373, 350,
	348, 366, 361, 374, 353, 358, 355, 363,
	370, 351, 387, 372, 384, 363, 375, 359,
	309, 407, 206, 342, 376, 219, 408, 343,
	335, 350, 353, 162, 218, 407, 342, 348,
	375, 290, 136, 134, 172, 172, 402, 305};

const int bispo_score_eg[64] = {
	224, 273, 265, 296, 276, 288, 262, 223,
	243, 275, 277, 293, 306, 267, 286, 267,
	287, 301, 316, 323, 326, 297, 290, 300,
	251, 330, 325, 336, 330, 306, 299, 296,
	263, 336, 327, 340, 341, 342, 308, 284,
	327, 313, 390, 346, 328, 387, 309, 352,
	287, 324, 318, 365, 358, 298, 309, 305,
	278, 315, 346, 348, 352, 334, 322, 276};

const int torre_score_mg[64] = {
	455, 458, 468, 489, 498, 470, 413, 431,
	410, 447, 417, 434, 469, 488, 439, 349,
	404, 426, 423, 423, 464, 447, 454, 421,
	428, 420, 461, 471, 469, 444, 457, 425,
	445, 443, 471, 505, 517, 507, 430, 466,
	460, 526, 488, 518, 530, 527, 560, 524,
	497, 483, 524, 517, 555, 570, 524, 558,
	507, 529, 519, 431, 427, 418, 539, 534};

const int torre_score_eg[64] = {
	494, 486, 492, 489, 478, 503, 502, 468,
	447, 450, 481, 491, 458, 469, 475, 453,
	461, 489, 468, 475, 471, 470, 490, 454,
	491, 509, 496, 483, 480, 484, 493, 488,
	508, 508, 515, 495, 492, 505, 521, 488,
	531, 503, 517, 518, 517, 506, 491, 484,
	535, 541, 519, 535, 519, 502, 517, 520,
	523, 522, 498, 533, 545, 546, 516, 527};

const int dama_score_mg[64] = {
	1016, 983, 1004, 1036, 1000, 976, 926, 974,
	941, 983, 1035, 1029, 1039, 1042, 1015, 980,
	988, 1020, 1013, 1010, 1019, 1015, 1025, 992,
	1010, 981, 1014, 1007, 1012, 1018, 1024, 997,
	1005, 983, 1023, 991, 1015, 1030, 1017, 1030,
	995, 995, 1018, 1026, 1031, 1078, 1058, 1073,
	986, 969, 1002, 1016, 991, 1098, 1057, 1093,
	1044, 1026, 1054, 1034, 1081, 1106, 1147, 1082};

const int dama_score_eg[64] = {
	869, 864, 875, 877, 879, 880, 844, 863,
	864, 872, 884, 891, 901, 902, 846, 903,
	893, 889, 950, 942, 886, 948, 941, 925,
	901, 977, 940, 998, 967, 974, 960, 990,
	920, 979, 994, 1004, 1030, 1026, 994, 963,
	927, 957, 975, 1001, 1016, 995, 959, 950,
	952, 996, 989, 1015, 1017, 968, 995, 957,
	729, 849, 871, 872, 913, 845, 754, 858};

const int rei_score_mg[64] = {
	-15, 36, 12, -54, 8, -28, 24, 14,
	1, 7, -8, -64, -43, -16, 9, 8,
	-14, -14, -22, -46, -44, -30, -15, -27,
	-49, -1, -27, -39, -46, -44, -33, -51,
	-17, -20, -12, -27, -30, -25, -14, -36,
	-9, 24, 2, -16, -20, 6, 22, -22,
	29, -1, -20, -7, -8, -4, -38, -29,
	-65, 23, 16, -15, -56, -34, 2, 13};

const int rei_score_eg[64] = {
	-53, -34, -21, -11, -28, -14, -24, -43,
	-27, -11, 4, 13, 14, 4, -5, -17,
	-19, -3, 11, 21, 23, 16, 7, -9,
	-18, -4, 21, 24, 27, 23, 9, -11,
	-8, 22, 24, 27, 26, 33, 26, 3,
	10, 17, 23, 15, 20, 45, 44, 13,
	-12, 17, 14, 17, 17, 38, 23, 11,
	-74, -35, -18, -18, -11, 15, 4, -17};

const int peao_passado_score[64] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	60, 60, 60, 60, 60, 60, 60, 60,
	30, 30, 30, 30, 30, 30, 30, 30,
	15, 15, 15, 15, 15, 15, 15, 15,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	0, 0, 0, 0, 0, 0, 0, 0};

};

#endif