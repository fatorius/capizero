/*
Nesse arquivo estão definidos os valores usados na avaliação de 
posições da engine, o valor absoluto de casa peça, os valores dos
adicionais para peão e torre, e os valores de cada casa para as peças
*/

#ifndef VALUES
#define VALUES

namespace Values{
	// VALORES DAS PEÇAS
	#define VALOR_PEAO 100
	#define VALOR_CAVALO 300
	#define VALOR_BISPO 315
	#define VALOR_TORRE 500
	#define VALOR_DAMA 900
	#define VALOR_REI 10000

	#define VALOR_PEAO_MG    0
	#define VALOR_PEAO_EG    0
	#define VALOR_CAVALO_MG  0
	#define VALOR_CAVALO_EG  0
	#define VALOR_BISPO_MG   0
	#define VALOR_BISPO_EG   0
	#define VALOR_TORRE_MG   0
	#define VALOR_TORRE_EG   0
	#define VALOR_DAMA_MG    0
	#define VALOR_DAMA_EG    0

	const int pieces_valor[6] =
	{
		VALOR_PEAO, VALOR_CAVALO, VALOR_BISPO, VALOR_TORRE, VALOR_DAMA, VALOR_REI
	};


	// VALORES DAS ESTRUTURAS
	#define EN_PASSANT_SCORE 10
	#define ISOLADO_SCORE 10

	#define COLUNA_SEMI_ABERTA_BONUS 11
	#define COLUNA_ABERTA_BONUS 40

	#define BISHOP_PAIR_MG 15
	#define BISHOP_PAIR_EG 45

	// King safety. For each side L we count L's attacks landing in the
	// 9-square zone around the enemy king (king + 8 neighbors), weighted
	// per attacker type. The accumulated "pressao" gets a quadratic mg
	// bonus: bonus = pressao² / KS_SCALE. Quadratic so the second/third
	// attacker matters disproportionately more than the first (one piece
	// near the king is fine, three pieces is mate). Mg-only because heavy
	// pieces come off in the endgame and the king becomes an active piece,
	// where this term would invert in motivation. Initial weights are
	// SF-classical-era literature defaults — Texel tuner adjusts them.
	#define KS_WEIGHT_C 20
	#define KS_WEIGHT_B 21
	#define KS_WEIGHT_T 41
	#define KS_WEIGHT_D 81
	#define KS_SCALE   256

	#define PHASE_PEAO    0
	#define PHASE_CAVALO  1
	#define PHASE_BISPO   1
	#define PHASE_TORRE   2
	#define PHASE_DAMA    4
	#define PHASE_REI     0
	#define PHASE_MAX    24

	const int mobilidade_cavalo_mg[9] = {
		-38, -13,  -1,   2,   5,   7,   7,   9,  11
	};
	const int mobilidade_cavalo_eg[9] = {
		-33, -10,  -1,   2,   5,   8,  11,  13,  15
	};
	const int mobilidade_bispo_mg[14] = {
		-23, -11,  -4,  -2,   1,   3,   9,   9,   8,   9,  10,  11,  13,  14
	};
	const int mobilidade_bispo_eg[14] = {
		-28, -17,  -5,   1,   6,   9,  12,  19,  21,  23,  23,  25,  23,  24
	};
	const int mobilidade_torre_mg[15] = {
		-23,  -8,  -2,  -2,   0,   3,   4,   5,   6,   7,   8,   8,   8,   8,   8
	};
	const int mobilidade_torre_eg[15] = {
		-33, -13,  -6,  -1,   7,  12,  17,  20,  22,  25,  27,  26,  26,  27,  28
	};
	const int mobilidade_dama_mg[28] = {
		-13,  -6,  -1,   1,   2,   0,   2,   2,   3,   7,   9,  10,  11,  12,
		 13,  14,  15,  16,  17,  17,  16,  17,  16,  13,  15,  13,  13,  17
	};
	const int mobilidade_dama_eg[28] = {
		-22, -10,  -6,   0,   0,   2,   4,   6,   8,  10,  16,  18,  20,  22,
		 24,  26,  28,  30,  31,  32,  33,  34,  35,  35,  32,  32,  32,  36
	};


	// REDUÇÕES E CONDIÇÕES
	#define REDUCAO_IID /4
	#define PROFUNDIDADE_CONDICAO_IID 5

	#define TAMANHO_JANELA_DE_PESQUISA 20
	#define MAX_ASPIRATION_FAILS 3

	// VALORES PARA ORDENAÇÃO DE LANCES
	#define SCORE_ROQUE        5000000
	#define SCORE_CAPTURAS_D   8000000
	#define SCORE_CONTRALANCE  9000000
	#define SCORE_KILLER_2     9500000
	#define SCORE_KILLER_1    10000000
	#define SCORE_CAPTURAS_V  50000000
	#define PONTUACAO_HASH   100000000

	#define SCORE_PROMO_Q_CAP  (SCORE_CAPTURAS_V + 1000000)
	#define SCORE_PROMO_Q      60000000
	#define SCORE_PROMO_N_CAP  18000000
	#define SCORE_PROMO_N      17000000

	#define REDUCAO_LMR 3

	#define R_NULL_LOW          2
	#define R_NULL_HIGH         3
	#define R_NULL_DEPTH_THRESH 6

	// Futility / reverse-futility pruning. Both fire only at low depths
	// (≤ FUTILITY_DEPTH_THRESH) and only at non-PV, non-check nodes.
	//   RFP: at node entry, if static_eval - margin >= beta → return early.
	//   FP : in move loop, skip quiet/non-promo moves when
	//        static_eval + margin + extra <= alpha (after at least one legal
	//        move has been searched, so we never return false stalemate).
	// Margins scale with depth — deeper depths get bigger margins because
	// the search has more room to swing the score.
	#define FUTILITY_DEPTH_THRESH      6
	#define FUTILITY_MARGIN_PER_PLY    100   // 1 pawn per ply
	#define FUTILITY_MARGIN_FP_EXTRA   50    // FP gets a little extra slack vs RFP

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

	const int peao_score_mg[64] = {
		   82,   82,   82,   82,   82,   82,   82,   82,
		   49,   79,   64,   61,   67,  104,  118,   62,
		   58,   79,   77,   74,   87,   87,  113,   72,
		   57,   82,   79,   92,   97,   90,   94,   59,
		   67,   93,   86,  101,  107,   96,   98,   61,
		   74,   87,  106,  111,  145,  139,  105,   60,
		  182,  218,  145,  179,  152,  210,  114,   73,
		   82,   82,   82,   82,   82,   82,   82,   82
	};
	const int peao_score_eg[64] = {
		   94,   94,   94,   94,   94,   94,   94,   94,
		  109,  100,  104,  106,  109,   96,   94,   89,
		  100,  100,   90,   97,   96,   91,   91,   88,
		  109,  105,   93,   89,   89,   88,   99,   95,
		  128,  117,  109,   97,   94,  100,  113,  113,
		  186,  192,  177,  159,  148,  145,  174,  176,
		  274,  265,  254,  230,  241,  225,  257,  279,
		   94,   94,   94,   94,   94,   94,   94,   94
	};

	const int cavalo_score_mg[64] = {
		  230,  318,  281,  306,  322,  311,  320,  312,
		  308,  286,  327,  336,  338,  353,  325,  320,
		  316,  330,  347,  349,  358,  352,  360,  323,
		  326,  343,  355,  352,  367,  355,  360,  331,
		  330,  356,  358,  388,  372,  404,  357,  361,
		  292,  395,  376,  400,  423,  468,  409,  383,
		  266,  298,  407,  375,  362,  401,  342,  322,
		  172,  250,  302,  290,  400,  242,  324,  231
	};
	const int cavalo_score_eg[64] = {
		  250,  232,  260,  268,  261,  265,  233,  216,
		  241,  263,  269,  278,  281,  259,  260,  239,
		  256,  280,  278,  298,  293,  276,  263,  257,
		  265,  277,  299,  308,  299,  300,  287,  265,
		  266,  286,  305,  305,  305,  294,  291,  265,
		  259,  263,  293,  292,  282,  274,  264,  242,
		  258,  275,  254,  281,  274,  257,  259,  231,
		  225,  245,  270,  255,  252,  256,  220,  184
	};

	const int bispo_score_mg[64] = {
		  334,  364,  353,  346,  350,  354,  328,  346,
		  367,  382,  379,  363,  370,  384,  396,  366,
		  367,  378,  378,  378,  377,  390,  381,  377,
		  357,  378,  376,  389,  399,  375,  377,  371,
		  359,  372,  382,  417,  400,  400,  374,  365,
		  351,  400,  406,  403,  402,  417,  404,  365,
		  338,  379,  349,  354,  393,  422,  381,  320,
		  338,  371,  285,  330,  338,  321,  374,  359
	};
	const int bispo_score_eg[64] = {
		  276,  288,  276,  290,  286,  279,  290,  282,
		  281,  277,  288,  294,  299,  286,  280,  268,
		  283,  292,  303,  305,  308,  298,  288,  284,
		  289,  300,  308,  314,  306,  305,  296,  286,
		  293,  308,  307,  308,  313,  308,  302,  301,
		  299,  291,  296,  298,  297,  305,  299,  303,
		  291,  293,  304,  287,  293,  286,  291,  285,
		  285,  278,  288,  291,  292,  290,  282,  275
	};

	const int torre_score_mg[64] = {
		  460,  466,  476,  492,  491,  486,  442,  449,
		  431,  459,  455,  466,  474,  486,  473,  408,
		  434,  454,  459,  458,  478,  475,  474,  446,
		  439,  453,  463,  474,  484,  472,  485,  456,
		  455,  468,  482,  503,  499,  512,  471,  459,
		  470,  498,  501,  511,  496,  524,  540,  495,
		  502,  507,  533,  537,  555,  546,  505,  523,
		  511,  518,  507,  526,  538,  488,  510,  522
	};
	const int torre_score_eg[64] = {
		  505,  515,  513,  509,  505,  501,  518,  494,
		  504,  504,  510,  512,  501,  501,  499,  511,
		  506,  510,  505,  509,  503,  498,  506,  498,
		  514,  519,  518,  514,  505,  508,  506,  503,
		  518,  517,  523,  514,  515,  515,  513,  516,
		  521,  518,  518,  515,  518,  511,  509,  511,
		  525,  525,  523,  521,  510,  514,  522,  517,
		  524,  524,  528,  525,  522,  526,  522,  519
	};

	const int dama_score_mg[64] = {
		 1022, 1009, 1018, 1035, 1012, 1000,  992,  977,
		  992, 1019, 1034, 1025, 1031, 1038, 1024, 1026,
		 1009, 1026, 1016, 1021, 1018, 1025, 1040, 1030,
		 1014, 1001, 1014, 1013, 1021, 1023, 1030, 1024,
		 1000, 1000, 1007, 1009, 1022, 1044, 1025, 1028,
		 1012, 1007, 1030, 1031, 1054, 1083, 1074, 1084,
		 1003,  984, 1018, 1024, 1011, 1084, 1055, 1081,
		  999, 1027, 1056, 1039, 1084, 1071, 1070, 1072
	};
	const int dama_score_eg[64] = {
		  901,  910,  912,  894,  929,  902,  914,  897,
		  916,  915,  904,  918,  918,  911,  901,  902,
		  918,  911,  951,  940,  943,  951,  944,  939,
		  918,  966,  953,  982,  969,  972,  977,  961,
		  941,  960,  958,  983,  995,  978,  995,  974,
		  918,  944,  943,  983,  985,  973,  957,  947,
		  921,  958,  968,  979,  996,  963,  968,  938,
		  929,  960,  960,  965,  965,  957,  948,  958
	};

	const int rei_score_mg[64] = {
		-15,  36,  12, -54,   8, -28,  24,  14,
		  1,   7,  -8, -64, -43, -16,   9,   8,
		-14, -14, -22, -46, -44, -30, -15, -27,
		-49,  -1, -27, -39, -46, -44, -33, -51,
		-17, -20, -12, -27, -30, -25, -14, -36,
		 -9,  24,   2, -16, -20,   6,  22, -22,
		 29,  -1, -20,  -7,  -8,  -4, -38, -29,
		-65,  23,  16, -15, -56, -34,   2,  13
	};
	const int rei_score_eg[64] = {
		-53, -34, -21, -11, -28, -14, -24, -43,
		-27, -11,   4,  13,  14,   4,  -5, -17,
		-19,  -3,  11,  21,  23,  16,   7,  -9,
		-18,  -4,  21,  24,  27,  23,   9, -11,
		 -8,  22,  24,  27,  26,  33,  26,   3,
		 10,  17,  23,  15,  20,  45,  44,  13,
		-12,  17,  14,  17,  17,  38,  23,  11,
		-74, -35, -18, -18, -11,  15,   4, -17
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
};

#endif