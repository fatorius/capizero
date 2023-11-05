import chess
import chess.pgn
from random import randint


def escolher_abertura_aleatoria(pgn):
    n = randint(0, 9)

    tabuleiro = chess.Board()

    if n == 0:
        node = pgn.add_variation(chess.Move.from_uci("e2e4"))
        tabuleiro.push(chess.Move.from_uci("e2e4"))

        node = node.add_variation(chess.Move.from_uci("e7e5"))
        tabuleiro.push(chess.Move.from_uci("e7e5"))

        return [node, tabuleiro]
    elif n == 1:
        node = pgn.add_variation(chess.Move.from_uci("e2e4"))
        tabuleiro.push(chess.Move.from_uci("e2e4"))

        node = node.add_variation(chess.Move.from_uci("c7c5"))
        tabuleiro.push(chess.Move.from_uci("c7c5"))

        return [node, tabuleiro]
    elif n == 2:
        node = pgn.add_variation(chess.Move.from_uci("e2e4"))
        tabuleiro.push(chess.Move.from_uci("e2e4"))

        node = node.add_variation(chess.Move.from_uci("e7e6"))
        tabuleiro.push(chess.Move.from_uci("e7e6"))

        return [node, tabuleiro]
    elif n == 3:
        node = pgn.add_variation(chess.Move.from_uci("e2e4"))
        tabuleiro.push(chess.Move.from_uci("e2e4"))

        node = node.add_variation(chess.Move.from_uci("c7c6"))
        tabuleiro.push(chess.Move.from_uci("c7c6"))

        return [node, tabuleiro]
    elif n == 4:
        node = pgn.add_variation(chess.Move.from_uci("e2e4"))
        tabuleiro.push(chess.Move.from_uci("e2e4"))

        node = node.add_variation(chess.Move.from_uci("g7g6"))
        tabuleiro.push(chess.Move.from_uci("g7g6"))

        return [node, tabuleiro]
    elif n == 5:
        node = pgn.add_variation(chess.Move.from_uci("d2d4"))
        tabuleiro.push(chess.Move.from_uci("d2d4"))

        node = node.add_variation(chess.Move.from_uci("d7d5"))
        tabuleiro.push(chess.Move.from_uci("d7d5"))

        return [node, tabuleiro]
    elif n == 6:
        node = pgn.add_variation(chess.Move.from_uci("d2d4"))
        tabuleiro.push(chess.Move.from_uci("d2d4"))

        node = node.add_variation(chess.Move.from_uci("g8f6"))
        tabuleiro.push(chess.Move.from_uci("g8f6"))

        return [node, tabuleiro]
    elif n == 7:
        node = pgn.add_variation(chess.Move.from_uci("c2c4"))
        tabuleiro.push(chess.Move.from_uci("c2c4"))

        node = node.add_variation(chess.Move.from_uci("c7c5"))
        tabuleiro.push(chess.Move.from_uci("c7c5"))

        return [node, tabuleiro]
    elif n == 8:
        node = pgn.add_variation(chess.Move.from_uci("c2c4"))
        tabuleiro.push(chess.Move.from_uci("c2c4"))

        node = node.add_variation(chess.Move.from_uci("g8f6"))
        tabuleiro.push(chess.Move.from_uci("g8f6"))

        return [node, tabuleiro]
    else:
        node = pgn.add_variation(chess.Move.from_uci("c2c4"))
        tabuleiro.push(chess.Move.from_uci("c2c4"))

        node = node.add_variation(chess.Move.from_uci("e7e5"))
        tabuleiro.push(chess.Move.from_uci("e7e5"))

        return [node, tabuleiro]










