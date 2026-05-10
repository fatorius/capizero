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

	#define PHASE_PEAO    0
	#define PHASE_CAVALO  1
	#define PHASE_BISPO   1
	#define PHASE_TORRE   2
	#define PHASE_DAMA    4
	#define PHASE_REI     0
	#define PHASE_MAX    24

	// Tapered mobility tables, indexed by attack-count (popcount of the
	// piece's attack bitboard with own pieces masked out). Stored here as
	// parallel mg/eg int arrays — `Eval::init_eval_tables` packs them into
	// `Eval::mobilidade_*` Score tables at startup. Curve shape: brutal
	// negative at trapped (low index), near-linear in the middle, saturating
	// at the top. First-pass literature values; will be Texel-tuned later.
	//   Knight max attack-count = 8, bishop = 13, rook = 14, queen = 27.
	const int mobilidade_cavalo_mg[9] = {
		-39, -14,  -2,   1,   4,   7,   8,  10,  12
	};
	const int mobilidade_cavalo_eg[9] = {
		-34, -11,  -2,   1,   4,   7,  10,  12,  14
	};
	const int mobilidade_bispo_mg[14] = {
		-24, -11,  -3,  -1,   2,   4,   8,  10,   9,  10,  11,  11,  12,  13
	};
	const int mobilidade_bispo_eg[14] = {
		-29, -16,  -4,   2,   7,  10,  13,  18,  20,  22,  23,  24,  24,  25
	};
	const int mobilidade_torre_mg[15] = {
		-24,  -9,  -2,  -1,   1,   3,   5,   6,   7,   8,   9,   9,   9,   9,   9
	};
	const int mobilidade_torre_eg[15] = {
		-34, -14,  -5,   0,   6,  11,  16,  19,  22,  24,  26,  26,  27,  28,  29
	};
	const int mobilidade_dama_mg[28] = {
		-14,  -7,  -1,   1,   2,   1,   2,   3,   4,   6,   8,   9,  10,  11,
		 12,  13,  14,  15,  16,  16,  16,  16,  16,  14,  14,  14,  14,  16
	};
	const int mobilidade_dama_eg[28] = {
		-21, -11,  -5,   1,   1,   3,   5,   7,   9,  11,  15,  17,  19,  21,
		 23,  25,  27,  29,  30,  31,  32,  33,  34,  34,  33,  33,  33,  35
	};


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

	// Promotion move scoring. Queen above MVV/LVA so it is always tried first,
	// knight moderate (tactical value). R/B are not generated.
	#define SCORE_PROMO_Q_CAP  (SCORE_CAPTURAS_V + 1000000)
	#define SCORE_PROMO_Q      60000000
	#define SCORE_PROMO_N_CAP  18000000
	#define SCORE_PROMO_N      17000000

	#define REDUCAO_LMR 3

	// Null-move pruning depth reduction. Dynamic R: heavier pruning at deep
	// depths where the search tree is huge and the marginal cost of missing
	// tactics is offset by the breadth gain; lighter pruning near the leaves
	// where the reduced-depth subsearch needs to retain enough resolution
	// to be informative. With R_NULL_HIGH = 3 and the threshold at 6, a
	// depth-6 null-move recurses at depth 2 (still beats qsearch by 2 plies).
	#define R_NULL_LOW          2
	#define R_NULL_HIGH         3
	#define R_NULL_DEPTH_THRESH 6

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
		   48,   80,   63,   60,   67,  105,  119,   61,
		   57,   79,   77,   73,   86,   86,  114,   71,
		   56,   81,   78,   93,   98,   89,   93,   58,
		   68,   94,   87,  102,  106,   95,   98,   60,
		   75,   88,  107,  112,  146,  139,  106,   61,
		  181,  217,  144,  178,  151,  209,  115,   72,
		   82,   82,   82,   82,   82,   82,   82,   82
	};
	const int peao_score_eg[64] = {
		   94,   94,   94,   94,   94,   94,   94,   94,
		  108,  101,  103,  105,  108,   95,   95,   88,
		   99,  101,   89,   96,   95,   90,   92,   87,
		  108,  104,   92,   88,   88,   87,   98,   94,
		  127,  117,  108,   98,   93,   99,  112,  112,
		  187,  193,  178,  160,  149,  146,  175,  177,
		  273,  266,  253,  229,  242,  226,  258,  280,
		   94,   94,   94,   94,   94,   94,   94,   94
	};

	const int cavalo_score_mg[64] = {
		  231,  317,  280,  305,  321,  310,  319,  313,
		  308,  285,  326,  335,  337,  354,  324,  319,
		  315,  329,  348,  348,  357,  353,  361,  322,
		  325,  342,  354,  351,  366,  355,  359,  330,
		  329,  355,  357,  389,  373,  405,  356,  360,
		  291,  396,  375,  401,  422,  467,  409,  382,
		  265,  297,  408,  374,  361,  400,  343,  321,
		  171,  249,  302,  289,  399,  241,  323,  230
	};
	const int cavalo_score_eg[64] = {
		  251,  231,  259,  267,  260,  264,  232,  217,
		  240,  262,  270,  277,  280,  260,  259,  238,
		  257,  279,  279,  297,  292,  277,  262,  258,
		  264,  276,  298,  307,  298,  299,  286,  264,
		  265,  285,  304,  304,  304,  293,  290,  264,
		  258,  262,  292,  291,  281,  273,  263,  241,
		  257,  274,  255,  280,  273,  257,  258,  230,
		  224,  244,  269,  254,  251,  255,  219,  183
	};

	const int bispo_score_mg[64] = {
		  333,  363,  352,  345,  351,  353,  327,  345,
		  368,  381,  380,  364,  371,  385,  397,  365,
		  366,  379,  379,  379,  378,  391,  382,  376,
		  358,  379,  377,  390,  400,  376,  376,  370,
		  360,  371,  383,  416,  401,  401,  373,  364,
		  350,  401,  407,  404,  401,  416,  403,  364,
		  339,  380,  348,  353,  394,  423,  382,  319,
		  337,  370,  284,  329,  339,  322,  373,  358
	};
	const int bispo_score_eg[64] = {
		  275,  288,  275,  291,  287,  280,  291,  281,
		  282,  278,  289,  295,  300,  287,  281,  269,
		  284,  293,  304,  306,  309,  299,  289,  283,
		  290,  300,  309,  315,  305,  306,  295,  287,
		  293,  307,  308,  307,  312,  308,  301,  300,
		  300,  290,  296,  297,  296,  304,  298,  302,
		  290,  293,  303,  286,  293,  285,  292,  284,
		  284,  277,  287,  290,  291,  289,  281,  274
	};

	const int torre_score_mg[64] = {
		  459,  465,  477,  493,  492,  485,  441,  450,
		  432,  460,  456,  467,  475,  487,  472,  407,
		  433,  453,  460,  459,  479,  476,  473,  445,
		  440,  452,  464,  475,  485,  471,  484,  455,
		  454,  467,  483,  504,  500,  513,  470,  458,
		  471,  497,  502,  512,  495,  523,  539,  494,
		  503,  508,  534,  538,  556,  545,  504,  522,
		  510,  519,  508,  527,  539,  487,  509,  521
	};
	const int torre_score_eg[64] = {
		  504,  515,  514,  510,  506,  500,  517,  493,
		  505,  505,  511,  513,  502,  502,  500,  510,
		  507,  511,  506,  510,  504,  499,  505,  497,
		  515,  518,  519,  515,  506,  507,  505,  502,
		  517,  516,  524,  514,  514,  514,  512,  515,
		  520,  519,  518,  516,  517,  510,  508,  510,
		  524,  525,  524,  522,  509,  514,  521,  516,
		  525,  523,  529,  526,  523,  525,  521,  518
	};

	const int dama_score_mg[64] = {
		 1023, 1008, 1017, 1036, 1011,  999,  993,  976,
		  991, 1018, 1035, 1026, 1032, 1039, 1023, 1025,
		 1010, 1026, 1015, 1022, 1019, 1026, 1039, 1030,
		 1015, 1000, 1015, 1014, 1022, 1022, 1029, 1023,
		  999,  999, 1008, 1010, 1023, 1043, 1024, 1027,
		 1013, 1008, 1031, 1032, 1055, 1082, 1073, 1083,
		 1002,  985, 1019, 1025, 1010, 1083, 1054, 1080,
		  998, 1026, 1055, 1038, 1085, 1070, 1069, 1071
	};
	const int dama_score_eg[64] = {
		  902,  909,  913,  894,  930,  903,  915,  896,
		  915,  914,  905,  919,  919,  912,  901,  903,
		  919,  910,  952,  941,  944,  952,  945,  940,
		  919,  965,  954,  982,  968,  971,  976,  960,
		  940,  959,  959,  982,  994,  977,  994,  973,
		  917,  943,  944,  984,  984,  972,  956,  946,
		  920,  957,  969,  978,  995,  962,  967,  937,
		  928,  959,  959,  964,  964,  956,  947,  957
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