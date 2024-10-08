#include "eval.h"

#include "bitboard.h"
#include "consts.h"
#include "game.h"


int Eval::score_casas[LADOS][TIPOS_DE_PIECES][CASAS_DO_TABULEIRO];
int Eval::reis_score_finais[LADOS][CASAS_DO_TABULEIRO];
int Eval::passados[LADOS][CASAS_DO_TABULEIRO];

int Eval::peao_mat[LADOS];
int Eval::piece_mat[LADOS];

int peao_ala_da_dama[LADOS],peao_ala_do_rei[LADOS];

void Eval::init_eval_tables(){
    for (int x = 0; x < CASAS_DO_TABULEIRO; x++){
        score_casas[BRANCAS][P][x] = Values::peao_score[x] + VALOR_PEAO;
        score_casas[BRANCAS][C][x] = Values::cavalo_score[x] + VALOR_CAVALO;
        score_casas[BRANCAS][B][x] = Values::bispo_score[x] + VALOR_BISPO;
        score_casas[BRANCAS][T][x] = Values::torre_score[x] + VALOR_TORRE;
        score_casas[BRANCAS][D][x] = Values::dama_score[x] + VALOR_DAMA;
        score_casas[BRANCAS][R][x] = Values::rei_score[x];

        score_casas[PRETAS][P][x] = Values::peao_score[Consts::flip[x]] + VALOR_PEAO;
        score_casas[PRETAS][C][x] = Values::cavalo_score[Consts::flip[x]] + VALOR_CAVALO;
        score_casas[PRETAS][B][x] = Values::bispo_score[Consts::flip[x]] + VALOR_BISPO;
        score_casas[PRETAS][T][x] = Values::torre_score[Consts::flip[x]] + VALOR_TORRE;
        score_casas[PRETAS][D][x] = Values::dama_score[Consts::flip[x]] + VALOR_DAMA;
        score_casas[PRETAS][R][x] = Values::rei_score[Consts::flip[x]];
        
        reis_score_finais[BRANCAS][x] = Values::rei_finais_score[x] - score_casas[BRANCAS][R][x];
        reis_score_finais[PRETAS][x] = Values::rei_finais_score[x] - score_casas[PRETAS][R][x];

        passados[BRANCAS][x] = Values::peao_passado_score[Consts::flip[x]];
        passados[PRETAS][x] = Values::peao_passado_score[x];
    }
}

void Eval::atualizar_materiais(){
    int cor;

    peao_mat[BRANCAS] = 0;
    peao_mat[PRETAS] = 0;

    piece_mat[BRANCAS] = 0;
    piece_mat[PRETAS] = 0;

    for (int casa = 0; casa < CASAS_DO_TABULEIRO; casa++){
        if (Bitboard::tabuleiro[casa] < VAZIO){
            if (Bitboard::bit_lados[BRANCAS] & Bitboard::mask[casa]){
                cor = BRANCAS;
            }
            else{
                cor = PRETAS;
            }

            if (Bitboard::tabuleiro[casa] == P){
                peao_mat[cor] += VALOR_PEAO;
            }
            else{
                piece_mat[cor] += Values::pieces_valor[Bitboard::tabuleiro[casa]];
            }
        }
    }
}

int avaliar_peao(const int l, const int casa){
    int score = 0;
    int xl = l^1;

    if (!(Bitboard::mask_passados[l][casa] & Bitboard::bit_pieces[xl][P]) && !(Bitboard::mask_path[l][casa] & Bitboard::bit_pieces[l][P])){
        score += Eval::passados[l][casa];
    }

    if ((Bitboard::mask_isolados[casa] & Bitboard::bit_pieces[l][P]) == 0){
        score -= ISOLADO_SCORE;
    }

    peao_ala_do_rei[l] = Values::defesa_ala_do_rei[l][casa];
    peao_ala_da_dama[l] = Values::defesa_ala_da_dama[l][casa];

    return score;
}

int avaliar_torre(const int l, const int casa){
    int score = 0;

    if (!(Bitboard::mask_cols[casa] & Bitboard::bit_pieces[l][P])){
        score = COLUNA_SEMI_ABERTA_BONUS;

        if (!(Bitboard::mask_cols[casa] & Bitboard::bit_pieces[l^1][P])){
            return COLUNA_ABERTA_BONUS;
        }
    }

    return score;
}

int Eval::avaliar(){
    int score[LADOS] = {0, 0};

    peao_ala_da_dama[BRANCAS] = 0;
    peao_ala_do_rei[BRANCAS] = 0;
    peao_ala_da_dama[PRETAS] = 0;
    peao_ala_do_rei[PRETAS] = 0;

    Bitboard::u64 t1;
    int casa;

    for (int l = 0; l < LADOS; l++){
        
        t1 = Bitboard::bit_pieces[l][P];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][P][casa];
            score[l] += avaliar_peao(l, casa);
        }

        t1 = Bitboard::bit_pieces[l][C];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][C][casa];
        }

        t1 = Bitboard::bit_pieces[l][B];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][B][casa];
        }

        t1 = Bitboard::bit_pieces[l][T];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][T][casa];
            score[l] += avaliar_torre(l, casa);
        }

        t1 = Bitboard::bit_pieces[l][D];
        while (t1){
            casa = Bitboard::bitscan(t1);
            t1 &= Bitboard::not_mask[casa];

            score[l] += score_casas[l][D][casa];
        }
    }

    if (Bitboard::bit_pieces[PRETAS][D] == 0){
        score[BRANCAS] += reis_score_finais[BRANCAS][Bitboard::bitscan(Bitboard::bit_pieces[BRANCAS][R])];
    }
    else{
        if (Bitboard::bit_pieces[BRANCAS][R] & Bitboard::mask_ala_do_rei){
            score[BRANCAS] += peao_ala_do_rei[BRANCAS];
        }
        else if (Bitboard::bit_pieces[BRANCAS][R] & Bitboard::mask_ala_da_dama){
            score[BRANCAS] += peao_ala_da_dama[BRANCAS];
        }
    }

    if (Bitboard::bit_pieces[BRANCAS][D] == 0){
        score[PRETAS] += reis_score_finais[PRETAS][Bitboard::bitscan(Bitboard::bit_pieces[PRETAS][R])];
    }
    else {
        if (Bitboard::bit_pieces[PRETAS][R] & Bitboard::mask_ala_do_rei){
            score[PRETAS] += peao_ala_do_rei[PRETAS];
        }
        else if (Bitboard::bit_pieces[PRETAS][R] & Bitboard::mask_ala_da_dama){
            score[PRETAS] += peao_ala_da_dama[PRETAS];
        }
    }

    return score[Game::lado] - score[Game::xlado];
}