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

	// PeSTO tapered piece values — mg and eg material baked into score_casas.
	#define VALOR_PEAO_MG    82
	#define VALOR_PEAO_EG    94
	#define VALOR_CAVALO_MG  337
	#define VALOR_CAVALO_EG  281
	#define VALOR_BISPO_MG   365
	#define VALOR_BISPO_EG   297
	#define VALOR_TORRE_MG   477
	#define VALOR_TORRE_EG   512
	#define VALOR_DAMA_MG    1025
	#define VALOR_DAMA_EG    936

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
		-40, -15,  -3,   0,   3,   6,   9,  11,  13
	};
	const int mobilidade_cavalo_eg[9] = {
		-35, -12,  -3,   0,   3,   6,   9,  11,  13
	};
	const int mobilidade_bispo_mg[14] = {
		-25, -10,  -2,   0,   3,   5,   7,   9,  10,  11,  12,  12,  12,  12
	};
	const int mobilidade_bispo_eg[14] = {
		-30, -15,  -3,   3,   8,  11,  14,  17,  19,  21,  23,  24,  25,  26
	};
	const int mobilidade_torre_mg[15] = {
		-25, -10,  -3,   0,   2,   4,   6,   7,   8,   9,  10,  10,  10,  10,  10
	};
	const int mobilidade_torre_eg[15] = {
		-35, -15,  -5,   0,   5,  10,  15,  18,  21,  23,  25,  27,  28,  29,  30
	};
	const int mobilidade_dama_mg[28] = {
		-15,  -8,  -2,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,
		 11,  12,  13,  14,  15,  15,  15,  15,  15,  15,  15,  15,  15,  15
	};
	const int mobilidade_dama_eg[28] = {
		-20, -12,  -4,   0,   2,   4,   6,   8,  10,  12,  14,  16,  18,  20,
		 22,  24,  26,  28,  29,  30,  31,  32,  33,  34,  34,  34,  34,  34
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
		  0,   0,   0,   0,   0,   0,   0,   0,
		-35,  -1, -20, -23, -15,  24,  38, -22,
		-26,  -4,  -4, -10,   3,   3,  33, -12,
		-27,  -2,  -5,  12,  17,   6,  10, -25,
		-14,  13,   6,  21,  23,  12,  17, -23,
		 -6,   7,  26,  31,  65,  56,  25, -20,
		 98, 134,  61,  95,  68, 126,  34, -11,
		  0,   0,   0,   0,   0,   0,   0,   0
	};
	const int peao_score_eg[64] = {
		  0,   0,   0,   0,   0,   0,   0,   0,
		 13,   8,   8,  10,  13,   0,   2,  -7,
		  4,   7,  -6,   1,   0,  -5,  -1,  -8,
		 13,   9,  -3,  -7,  -7,  -8,   3,  -1,
		 32,  24,  13,   5,  -2,   4,  17,  17,
		 94, 100,  85,  67,  56,  53,  82,  84,
		178, 173, 158, 134, 147, 132, 165, 187,
		  0,   0,   0,   0,   0,   0,   0,   0
	};

	const int cavalo_score_mg[64] = {
		-105, -21, -58, -33, -17, -28, -19, -23,
		 -29, -53, -12,  -3,  -1,  18, -14, -19,
		 -23,  -9,  12,  10,  19,  17,  25, -16,
		 -13,   4,  16,  13,  28,  19,  21,  -8,
		  -9,  17,  19,  53,  37,  69,  18,  22,
		 -47,  60,  37,  65,  84, 129,  73,  44,
		 -73, -41,  72,  36,  23,  62,   7, -17,
		-167, -89, -34, -49,  61, -97, -15, -107
	};
	const int cavalo_score_eg[64] = {
		-29, -51, -23, -15, -22, -18, -50, -64,
		-42, -20, -10,  -5,  -2, -20, -23, -44,
		-23,  -3,  -1,  15,  10,  -3, -20, -22,
		-18,  -6,  16,  25,  16,  17,   4, -18,
		-17,   3,  22,  22,  22,  11,   8, -18,
		-24, -20,  10,   9,  -1,  -9, -19, -41,
		-25,  -8, -25,  -2,  -9, -25, -24, -52,
		-58, -38, -13, -28, -31, -27, -63, -99
	};

	const int bispo_score_mg[64] = {
		-33,  -3, -14, -21, -13, -12, -39, -21,
		  4,  15,  16,   0,   7,  21,  33,   1,
		  0,  15,  15,  15,  14,  27,  18,  10,
		 -6,  13,  13,  26,  34,  12,  10,   4,
		 -4,   5,  19,  50,  37,  37,   7,  -2,
		-16,  37,  43,  40,  35,  50,  37,  -2,
		-26,  16, -18, -13,  30,  59,  18, -47,
		-29,   4, -82, -37, -25, -42,   7,  -8
	};
	const int bispo_score_eg[64] = {
		-23,  -9, -23,  -5,  -9, -16,  -5, -17,
		-14, -18,  -7,  -1,   4,  -9, -15, -27,
		-12,  -3,   8,  10,  13,   3,  -7, -15,
		 -6,   3,  13,  19,   7,  10,  -3,  -9,
		 -3,   9,  12,   9,  14,  10,   3,   2,
		  2,  -8,   0,  -1,  -2,   6,   0,   4,
		 -8,  -4,   7, -12,  -3, -13,  -4, -14,
		-14, -21, -11,  -8,  -7,  -9, -17, -24
	};

	const int torre_score_mg[64] = {
		-19, -13,   1,  17,  16,   7, -37, -26,
		-44, -16, -20,  -9,  -1,  11,  -6, -71,
		-45, -25, -16, -17,   3,   0,  -5, -33,
		-36, -26, -12,  -1,   9,  -7,   6, -23,
		-24, -11,   7,  26,  24,  35,  -8, -20,
		 -5,  19,  26,  36,  17,  45,  61,  16,
		 27,  32,  58,  62,  80,  67,  26,  44,
		 32,  42,  32,  51,  63,   9,  31,  43
	};
	const int torre_score_eg[64] = {
		 -9,   2,   3,  -1,  -5, -13,   4, -20,
		 -6,  -6,   0,   2,  -9,  -9, -11,  -3,
		 -4,   0,  -5,  -1,  -7, -12,  -8, -16,
		  3,   5,   8,   4,  -5,  -6,  -8, -11,
		  4,   3,  13,   1,   2,   1,  -1,   2,
		  7,   7,   7,   5,   4,  -3,  -5,  -3,
		 11,  13,  13,  11,  -3,   3,   8,   3,
		 13,  10,  18,  15,  12,  12,   8,   5
	};

	const int dama_score_mg[64] = {
		 -1, -18,  -9,  10, -15, -25, -31, -50,
		-35,  -8,  11,   2,   8,  15,  -3,   1,
		-14,   2, -11,  -2,  -5,   2,  14,   5,
		 -9, -26,  -9, -10,  -2,  -4,   3,  -3,
		-27, -27, -16, -16,  -1,  17,  -2,   1,
		-13, -17,   7,   8,  29,  56,  47,  57,
		-24, -39,  -5,   1, -16,  57,  28,  54,
		-28,   0,  29,  12,  59,  44,  43,  45
	};
	const int dama_score_eg[64] = {
		-33, -28, -22, -43,  -5, -32, -20, -41,
		-22, -23, -30, -16, -16, -23, -36, -32,
		-16, -27,  15,   6,   9,  17,  10,   5,
		-18,  28,  19,  47,  31,  34,  39,  23,
		  3,  22,  24,  45,  57,  40,  57,  36,
		-20,   6,   9,  49,  47,  35,  19,   9,
		-17,  20,  32,  41,  58,  25,  30,   0,
		 -9,  22,  22,  27,  27,  19,  10,  20
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