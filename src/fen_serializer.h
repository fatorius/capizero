#ifndef FEN_SERIALIZER
#define FEN_SERIALIZER

namespace Fen {
    // Writes a FEN string for the current global engine state into `out`.
    // Caller owns the buffer; ≥90 bytes is safe for any legal position.
    // Reads: Bitboard::tabuleiro, Bitboard::bit_lados, Game::lado,
    // Game::roque, Game::cinquenta, Game::hply, Game::lista_do_jogo[hply-1].
    // Inverse of Update::setar_posicao for positions reachable via real play
    // (note: setar_posicao discards the FEN's fullmove counter, so a round-trip
    // through that function may shift the fullmove field).
    void serialize(char* out);
}

#endif
