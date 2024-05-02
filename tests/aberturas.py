import chess
import chess.pgn
from random import randint

aberturas = [
    ["e2e4", "e7e5"],
    ["e2e4", "c7c5"],
    ["e2e4", "e7e6"],
    ["e2e4", "c7c6"],
    ["e2e4", "g7g6"],
    ["d2d4", "d7d5"],
    ["d2d4", "g8f6"],
    ["c2c4", "c7c5"],
    ["c2c4", "g8f6"],
    ["c2c4", "e7e5"]
]

def escolher_abertura_aleatoria(pgn):
    tabuleiro = chess.Board()

    n = randint(0, len(aberturas))

    abertura_escolhida = aberturas[n]

    for lance in abertura_escolhida:
            node = pgn.add_variation(chess.Move.from_uci(lance))
            tabuleiro.push(chess.Move.from_uci(lance))

    return [node, tabuleiro]










