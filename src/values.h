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

	const int pieces_valor[6] =
		{
			VALOR_PEAO, VALOR_CAVALO, VALOR_BISPO, VALOR_TORRE, VALOR_DAMA, VALOR_REI};

// VALORES DAS ESTRUTURAS
#define EN_PASSANT_SCORE 10
#define ISOLADO_SCORE 10

#define COLUNA_SEMI_ABERTA_BONUS 11
#define COLUNA_ABERTA_BONUS 40

#define BISHOP_PAIR_MG 15
#define BISHOP_PAIR_EG 45

#define KS_WEIGHT_C 22
#define KS_WEIGHT_B 21
#define KS_WEIGHT_T 63
#define KS_WEIGHT_D 146
#define KS_SCALE 256

#define PHASE_PEAO 0
#define PHASE_CAVALO 1
#define PHASE_BISPO 1
#define PHASE_TORRE 2
#define PHASE_DAMA 4
#define PHASE_REI 0
#define PHASE_MAX 24

	const int mobilidade_cavalo_mg[9] = {
		-57, -43, -34, -27, -30, -34, -35, -37,
		-35};

	const int mobilidade_cavalo_eg[9] = {
		-116, -10, 1, 5, 18, 31, 34, 36,
		24};

	const int mobilidade_bispo_mg[14] = {
		-74, -62, -44, -38, -34, -31, -23, -22,
		-20, -23, -14, -4, 11, 72};

	const int mobilidade_bispo_eg[14] = {
		-69, -29, -33, -9, 4, 26, 26, 34,
		40, 41, 45, 49, 16, 15};

	const int mobilidade_torre_mg[15] = {
		-38, -29, -35, -26, -17, -8, -5, -3,
		-2, 5, 12, 18, 5, 4, 15};

	const int mobilidade_torre_eg[15] = {
		-26, -23, -33, -25, -3, -2, 1, 2,
		4, 1, -2, -8, -10, -18, -17};

	const int mobilidade_dama_mg[28] = {
		-28, -30, -27, -24, -17, -15, -13, -7,
		-6, 1, 1, 2, 9, 4, 1, 16,
		9, 8, 3, 15, 10, 25, 55, -16,
		36, 32, -84, -54};

	const int mobilidade_dama_eg[28] = {
		-5, -54, -39, -32, -8, -15, -15, -6,
		0, -8, 20, 26, 8, 22, 17, 7,
		19, -7, 15, -26, -30, -73, -63, -34,
		-33, -119, -148, -121};

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
	const int defesa_ala_da_dama[2][64] =
		{
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
			 0, 0, 0, 0, 0, 0, 0, 0}};

	const int defesa_ala_do_rei[2][64] =
		{
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
			 0, 0, 0, 0, 0, 0, 0, 0}};

	const int peao_score_mg[64] = {
		82, 82, 82, 82, 82, 82, 82, 82,
		75, 96, 73, 77, 75, 110, 152, 77,
		87, 91, 91, 98, 104, 103, 137, 93,
		78, 89, 91, 117, 111, 109, 112, 62,
		86, 106, 91, 119, 125, 109, 98, 69,
		98, 94, 109, 151, 161, 112, 144, 89,
		289, 232, 224, 255, 214, 261, 159, 152,
		82, 82, 82, 82, 82, 82, 82, 82};

	const int peao_score_eg[64] = {
		94, 94, 94, 94, 94, 94, 94, 94,
		150, 145, 141, 132, 127, 139, 123, 123,
		136, 139, 119, 116, 119, 113, 119, 119,
		147, 147, 131, 119, 114, 111, 135, 140,
		181, 163, 148, 133, 124, 126, 160, 167,
		247, 246, 234, 196, 185, 196, 218, 225,
		352, 340, 338, 296, 304, 320, 334, 360,
		94, 94, 94, 94, 94, 94, 94, 94};

	const int cavalo_score_mg[64] = {
		250, 285, 287, 245, 314, 263, 282, 276,
		264, 269, 292, 323, 316, 328, 296, 268,
		279, 316, 327, 343, 344, 340, 348, 280,
		305, 313, 348, 345, 342, 340, 350, 308,
		312, 339, 370, 366, 359, 406, 354, 353,
		252, 365, 303, 350, 368, 309, 384, 352,
		300, 289, 400, 187, 175, 391, 334, 311,
		207, 293, 278, 204, 448, 188, 368, 188};

	const int cavalo_score_eg[64] = {
		189, 198, 231, 265, 252, 266, 203, 170,
		235, 266, 265, 276, 279, 257, 237, 221,
		229, 260, 292, 320, 327, 295, 245, 238,
		258, 289, 319, 329, 330, 328, 293, 264,
		266, 297, 345, 339, 335, 315, 313, 271,
		302, 283, 363, 351, 333, 372, 260, 270,
		286, 308, 244, 382, 377, 285, 266, 252,
		284, 279, 308, 276, 239, 274, 265, 240};

	const int bispo_score_mg[64] = {
		281, 351, 351, 286, 346, 339, 295, 321,
		380, 367, 368, 351, 361, 369, 390, 361,
		364, 362, 366, 366, 367, 378, 376, 374,
		350, 337, 359, 377, 348, 356, 338, 382,
		370, 361, 385, 389, 381, 359, 368, 347,
		315, 435, 218, 359, 395, 229, 406, 352,
		299, 366, 351, 173, 257, 424, 335, 307,
		329, 290, 134, 199, 205, 133, 446, 372};

	const int bispo_score_eg[64] = {
		252, 276, 266, 290, 270, 278, 289, 274,
		228, 270, 289, 300, 302, 270, 274, 272,
		281, 306, 320, 317, 330, 291, 292, 273,
		276, 330, 320, 339, 325, 324, 314, 239,
		280, 340, 317, 325, 347, 334, 312, 325,
		334, 292, 391, 322, 316, 389, 323, 324,
		295, 292, 304, 339, 371, 305, 313, 330,
		264, 295, 328, 341, 303, 324, 287, 322};

	const int torre_score_mg[64] = {
		460, 457, 476, 491, 496, 471, 409, 428,
		407, 433, 419, 447, 462, 483, 426, 343,
		406, 442, 428, 421, 445, 458, 455, 423,
		435, 429, 430, 469, 447, 457, 481, 408,
		461, 439, 465, 503, 501, 507, 467, 475,
		487, 522, 517, 548, 509, 544, 552, 490,
		516, 482, 553, 561, 567, 556, 443, 542,
		503, 488, 480, 381, 376, 397, 518, 526};

	const int torre_score_eg[64] = {
		482, 500, 487, 487, 480, 496, 504, 475,
		437, 441, 478, 487, 469, 457, 466, 456,
		465, 457, 465, 479, 481, 450, 465, 461,
		479, 494, 515, 497, 486, 481, 478, 479,
		507, 524, 524, 497, 493, 499, 499, 499,
		522, 516, 514, 488, 515, 486, 495, 510,
		519, 542, 518, 520, 513, 505, 540, 526,
		525, 537, 501, 566, 568, 554, 541, 532};

	const int dama_score_mg[64] = {
		1000, 983, 1025, 1037, 1021, 957, 941, 981,
		953, 1015, 1033, 1026, 1032, 1032, 991, 982,
		989, 1016, 1008, 1010, 1011, 1014, 1027, 1005,
		1004, 993, 1004, 1001, 1000, 1009, 1009, 993,
		991, 996, 1034, 995, 1024, 1022, 1011, 1032,
		989, 1004, 1001, 1027, 1051, 1105, 1080, 1063,
		992, 964, 1008, 1004, 1005, 1099, 1057, 1099,
		1044, 1058, 1040, 974, 1068, 1104, 1167, 1113};

	const int dama_score_eg[64] = {
		868, 887, 841, 865, 859, 816, 821, 898,
		862, 897, 888, 904, 894, 856, 842, 882,
		926, 903, 956, 942, 943, 930, 925, 902,
		918, 956, 950, 1005, 983, 973, 987, 966,
		945, 952, 968, 1033, 1032, 1007, 1028, 942,
		950, 955, 978, 1012, 1000, 974, 982, 954,
		948, 972, 1004, 975, 1035, 975, 1019, 973,
		741, 837, 896, 925, 916, 910, 798, 787};

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

	const int peao_passado_score[64] =
		{
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