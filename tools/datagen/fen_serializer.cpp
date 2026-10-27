#include "fen_serializer.h"

#include <cstdio>
#include <cstdlib>

#include "consts.h"
#include "bitboard.h"
#include "game.h"
#include "estado.h"

void Fen::serialize(char* out) {
    int pos = 0;

    // 1. Piece placement: ranks 7 (rank 8 in algebraic) → 0, files 0 → 7.
    //    Square index = rank * 8 + file (matches the convention in
    //    Update::setar_posicao). Empty runs compress to a single digit.
    for (int rank = 7; rank >= 0; rank--) {
        int empty = 0;
        for (int file = 0; file < 8; file++) {
            const int sq = rank * 8 + file;
            const int piece = estado_principal.tabuleiro[sq];
            if (piece == VAZIO) {
                empty++;
                continue;
            }
            if (empty > 0) {
                out[pos++] = '0' + empty;
                empty = 0;
            }
            char c = Consts::piece_char[piece];
            // Consts::piece_char is uppercase by default. Lowercase for black.
            if (estado_principal.bit_lados[PRETAS] & Bitboard::mask[sq]) {
                c = c + ('a' - 'A');
            }
            out[pos++] = c;
        }
        if (empty > 0) {
            out[pos++] = '0' + empty;
        }
        if (rank > 0) {
            out[pos++] = '/';
        }
    }

    out[pos++] = ' ';

    // 2. Side to move.
    out[pos++] = (estado_principal.lado == BRANCAS) ? 'w' : 'b';

    out[pos++] = ' ';

    // 3. Castling rights, in canonical KQkq order.
    if (estado_principal.roque == 0) {
        out[pos++] = '-';
    } else {
        if (estado_principal.roque & BRANCAS_ROQUE_MENOR) out[pos++] = 'K';
        if (estado_principal.roque & BRANCAS_ROQUE_MAIOR) out[pos++] = 'Q';
        if (estado_principal.roque & PRETAS_ROQUE_MENOR)  out[pos++] = 'k';
        if (estado_principal.roque & PRETAS_ROQUE_MAIOR)  out[pos++] = 'q';
    }

    out[pos++] = ' ';

    // 4. En passant target. 
    bool ep_emitted = false;
    if (estado_principal.hply > 1) {
        const Game::jogo& last = estado_principal.lista_do_jogo[estado_principal.hply - 1];
        const int piece_at_dest = estado_principal.tabuleiro[last.destino];
        if (piece_at_dest == P && std::abs(last.destino - last.inicio) == 16) {
            const int ep_sq = (last.inicio + last.destino) / 2;
            const int ep_file = ep_sq & 7;
            const int ep_rank = ep_sq >> 3;
            out[pos++] = 'a' + ep_file;
            out[pos++] = '1' + ep_rank;
            ep_emitted = true;
        }
    }
    if (!ep_emitted) {
        out[pos++] = '-';
    }

    out[pos++] = ' ';

    // 5. Halfmove clock.
    pos += std::sprintf(out + pos, "%d", estado_principal.cinquenta);

    out[pos++] = ' ';

    // 6. Fullmove number. 
    pos += std::sprintf(out + pos, "%d", (estado_principal.hply + 1) / 2);

    out[pos] = '\0';
}
