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

	#define KS_WEIGHT_C 28
	#define KS_WEIGHT_B 37
	#define KS_WEIGHT_T 23
	#define KS_WEIGHT_D 36
	#define KS_SCALE   256

	#define PHASE_PEAO    0
	#define PHASE_CAVALO  1
	#define PHASE_BISPO   1
	#define PHASE_TORRE   2
	#define PHASE_DAMA    4
	#define PHASE_REI     0
	#define PHASE_MAX    24

	const int mobilidade_cavalo_mg[9] = {
		  -23,  -17,  -11,   -4,    1,    2,    0,   -1,
		    0
	};
	const int mobilidade_cavalo_eg[9] = {
		  -31,   12,   47,   46,   54,   66,   66,   69,
		   67
	};
	const int mobilidade_bispo_mg[14] = {
		  -56,  -36,  -27,  -22,  -14,   -6,   -2,   -1,
		    1,    1,    4,    6,   39,   27
	};
	const int mobilidade_bispo_eg[14] = {
		  -15,   28,   36,   43,   57,   64,   66,   74,
		   76,   79,   80,   79,   66,   63
	};
	const int mobilidade_torre_mg[15] = {
		  -56,  -46,  -41,  -33,  -31,  -25,  -27,  -20,
		  -20,  -13,  -11,  -18,  -13,    0,   29
	};
	const int mobilidade_torre_eg[15] = {
		   33,   56,   64,   73,   86,   85,   89,   81,
		   87,   84,   82,   89,   81,   73,   42
	};
	const int mobilidade_dama_mg[28] = {
		   23,    2,    4,   -5,   -6,   -6,   -1,   -6,
		   -4,    1,   -3,    0,   -2,    1,   -8,   -6,
		    5,    1,   12,   -4,    6,   29,   58,  112,
		  130,  174,   13,   17
	};
	const int mobilidade_dama_eg[28] = {
		 -179,   15,  -66,   46,  100,  104,   75,   97,
		  118,  119,  141,  144,  149,  152,  177,  172,
		  154,  166,  161,  178,  152,  136,  101,   31,
		   -4,  -47,   32,   36
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

	#define USE_LMR_TABLE        1   // 1 = tabela pré-computada, 0 = log() inline (A/B)
	#define LMR_TABLE_DEPTH      128
	#define LMR_TABLE_MOVES      256
	#define LMR_DIVISOR          1.75
	#define USE_LMR_PV           0   // nós PV de verdade (janela beta-alpha > 1) reduzem 1 ply a menos
	#define USE_LMR_HISTORY      0   // lances quietos com histórico alto sofrem 1 ply a menos de redução
	#define LMR_HISTORY_K        8   // limiar = LMR_HISTORY_K << profundidade (o bônus de histórico é 1 << profundidade)
	#define USE_LMR_CHECK_LESS   0   // lances que dão xeque sofrem 1 ply a menos de redução (não são isentos)

	#define R_NULL_LOW          2
	#define R_NULL_HIGH         3
	#define R_NULL_DEPTH_THRESH 6

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
		   54,   67,   65,   62,   64,   87,  104,   56,
		   59,   67,   74,   82,   83,   81,   98,   63,
		   55,   67,   80,  100,   97,   90,   80,   46,
		   47,   81,   90,  109,  113,   99,   86,   49,
		   78,   55,  106,  107,  126,  181,  136,   93,
		  267,  177,  193,  234,  252,  200,  136,  153,
		   82,   82,   82,   82,   82,   82,   82,   82
	};
	const int peao_score_eg[64] = {
		   94,   94,   94,   94,   94,   94,   94,   94,
		  143,  134,  128,  141,  136,  119,  116,  121,
		  135,  135,  115,  115,  114,  118,  118,  124,
		  148,  141,  128,  114,  111,  115,  131,  137,
		  173,  161,  125,  119,  113,  118,  136,  151,
		  207,  205,  174,  154,  136,  123,  160,  184,
		  321,  338,  319,  253,  215,  274,  281,  300,
		   94,   94,   94,   94,   94,   94,   94,   94
	};

	const int cavalo_score_mg[64] = {
		  234,  304,  327,  302,  332,  332,  320,  321,
		  305,  260,  324,  330,  336,  340,  300,  324,
		  298,  329,  336,  351,  353,  348,  345,  319,
		  328,  328,  355,  360,  370,  365,  353,  332,
		  331,  345,  369,  396,  370,  385,  359,  380,
		  310,  390,  371,  406,  465,  460,  408,  293,
		  289,  301,  377,  418,  380,  432,  285,  342,
		  193,  279,  313,  325,  362,   78,  311,  179
	};
	const int cavalo_score_eg[64] = {
		  204,  265,  287,  313,  307,  295,  294,  208,
		  298,  327,  278,  305,  295,  284,  296,  312,
		  281,  283,  306,  323,  319,  313,  291,  298,
		  292,  323,  328,  334,  315,  324,  305,  298,
		  310,  310,  312,  314,  328,  315,  307,  274,
		  330,  301,  322,  294,  288,  274,  309,  271,
		  300,  291,  279,  291,  257,  237,  307,  235,
		  195,  308,  306,  273,  253,  358,  330,  262
	};

	const int bispo_score_mg[64] = {
		  327,  362,  360,  361,  383,  338,  343,  388,
		  347,  377,  374,  359,  358,  378,  380,  379,
		  374,  367,  371,  364,  370,  364,  376,  357,
		  363,  368,  364,  386,  374,  370,  352,  374,
		  346,  361,  377,  390,  373,  390,  371,  344,
		  351,  374,  389,  422,  376,  437,  384,  375,
		  329,  353,  356,  364,  373,  328,  357,  314,
		  287,  327,  279,  276,  208,  288,  281,  329
	};
	const int bispo_score_eg[64] = {
		  302,  286,  319,  297,  290,  330,  318,  287,
		  325,  311,  317,  326,  323,  291,  307,  295,
		  319,  327,  332,  329,  342,  340,  295,  314,
		  289,  315,  338,  332,  336,  326,  318,  286,
		  320,  337,  326,  347,  333,  325,  329,  325,
		  320,  332,  323,  299,  336,  312,  325,  324,
		  299,  328,  317,  321,  321,  332,  330,  267,
		  359,  296,  309,  347,  328,  305,  309,  379
	};

	const int torre_score_mg[64] = {
		  442,  443,  448,  458,  465,  457,  444,  459,
		  411,  432,  453,  468,  453,  459,  473,  418,
		  425,  435,  446,  438,  450,  426,  455,  454,
		  445,  407,  432,  468,  448,  462,  466,  453,
		  449,  457,  475,  480,  474,  458,  504,  456,
		  453,  483,  496,  499,  548,  540,  562,  506,
		  476,  469,  530,  558,  549,  544,  442,  475,
		  503,  512,  643,  506,  519,  527,  506,  438
	};
	const int torre_score_eg[64] = {
		  553,  550,  545,  535,  528,  541,  536,  529,
		  538,  542,  528,  515,  524,  531,  496,  534,
		  563,  528,  539,  522,  532,  544,  534,  520,
		  539,  580,  555,  532,  545,  541,  534,  522,
		  549,  550,  540,  533,  544,  551,  521,  546,
		  559,  547,  540,  529,  506,  511,  515,  518,
		  565,  556,  547,  526,  527,  524,  565,  562,
		  551,  555,  487,  541,  544,  548,  542,  575
	};

	const int dama_score_mg[64] = {
		 1030, 1036, 1057, 1043, 1045, 1057, 1004, 1023,
		  972, 1023, 1048, 1044, 1045, 1062, 1077, 1017,
		 1021, 1031, 1015, 1031, 1035, 1033, 1034, 1012,
		 1044, 1011, 1034, 1027, 1027, 1036, 1022, 1028,
		 1019,  996, 1013,  990, 1011, 1036, 1040, 1043,
		 1011, 1011,  987, 1044, 1029, 1013, 1048, 1036,
		 1012,  983, 1020, 1017, 1049, 1050,  975, 1021,
		  953, 1035,  996, 1014, 1028, 1144, 1088, 1016
	};
	const int dama_score_eg[64] = {
		  868,  892,  886,  931,  910,  831,  913,  982,
		  960,  913,  924,  955,  953,  940,  829,  935,
		  941,  955, 1015,  958,  978,  979,  995,  990,
		  880, 1030,  994, 1065, 1030, 1016, 1039,  983,
		  977, 1021, 1012, 1078, 1080, 1002,  991,  957,
		  978,  983, 1088, 1017, 1052, 1072, 1047,  971,
		  979, 1048, 1038, 1027, 1031, 1018, 1108, 1019,
		 1015,  970, 1026, 1002,  956,  925,  935, 1043
	};

	const int rei_score_mg[64] = {
		  -27,   11,   -3,  -57,  -19,  -33,   62,   47,
		    0,    9,  -17,  -49,  -42,  -12,   36,   38,
		   34,  -32,  -41,  -80,  -66,  -74,    2,  -14,
		  -18, -156,  -48,  -27, -145,  -35,  -92,  -88,
		  -95, -138, -145, -182,  -95,  -63, -101,  -89,
		  -36,  -81, -107,  -66,   15,  -11,  -34,   19,
		  120,   95,   20,  150,   89,  114,   94,  -22,
		    2,   95,  116,  -27,  -53,   26,   57,   24
	};
	const int rei_score_eg[64] = {
		  -41,  -39,  -13,   -4,  -23,  -20,  -62,  -88,
		  -22,   -5,   12,   30,   26,    7,  -17,  -40,
		  -30,    9,   29,   39,   43,   42,    3,  -10,
		  -18,   47,   47,   37,   59,   45,   43,    5,
		    8,   53,   56,   63,   74,   66,   68,   41,
		   43,   63,   55,   37,   69,   90,   78,   44,
		  -83,  -29,   18,   44,   10,   52,   95,   21,
		 -132,  -96, -102,  -41,   84,    4,    0,  -75
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