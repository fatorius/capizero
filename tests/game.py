import chess
import chess.engine
import time
import os
import sys
import math

RELEASE_RATING = sys.argv[1]
ROUNDS = sys.argv[2]

TEMPO_EM_SEGUNDOS = 1

LOCAL_SCORE = 0
RELEASE_SCORE = 0


def clear_terminal():
    if os.name == 'posix':  # For Unix/Linux/Mac
        os.system('clear')
    elif os.name == 'nt':   # For Windows
        os.system('cls')
    else:
        print("Unsupported operating system. Cannot clear the terminal.")


def termination_to_string(termination):
    if termination == chess.Termination.CHECKMATE:
        return "Xeque-mate"
    elif termination == chess.Termination.STALEMATE:
        return "Afogamento"
    elif termination == chess.Termination.INSUFFICIENT_MATERIAL:
        return "Material insuficiente"
    elif termination == chess.Termination.SEVENTYFIVE_MOVES:
        return "75 lances sem progresso"
    elif termination == chess.Termination.FIVEFOLD_REPETITION:
        return "Repetição de 5 lances"
    elif termination == chess.Termination.FIFTY_MOVES:
        return "50 lances sem progresso"
    else:
        return "Repetição de 3 lances"

class TestGame:
    def __init__(self, white: str, black: str, no: int):
        self.jogo_no = no
        self.white = white
        self.black = black
        self.white_engine = chess.engine.SimpleEngine.popen_xboard("./" + self.white)
        self.black_engine = chess.engine.SimpleEngine.popen_xboard("./" + self.black)
        self.tabuleiro = chess.Board()
        self.lado = "BRANCO"
        self.ultimo_lance = ""
        self.quantidade_de_lances = 1
        self.local_score = 0
        self.release_score = 0
        self.info = None

    def print_game(self):
        clear_terminal()
        print(f"Jogo {self.jogo_no} : {self.white} - {self.black}")
        print("-" * 30)
        print(self.tabuleiro)
        print("-" * 30)

        if self.lado == "BRANCO":
            print(f"{self.white}: {self.quantidade_de_lances}. {self.ultimo_lance} ({self.info['score'].relative.score()})")
        else:
            print(f"{self.black}: {self.quantidade_de_lances}... {self.ultimo_lance} ({self.info['score'].relative.score() * -1})")

        print(f"Profundidade {self.info['depth']} em {self.info['time']} segundos")

        print("\n\n\n")

    def play(self):
        while not self.tabuleiro.is_game_over():
            self.lado = "BRANCO"
            result = self.white_engine.play(self.tabuleiro, chess.engine.Limit(time=TEMPO_EM_SEGUNDOS))
            self.ultimo_lance = result.move
            self.info = result.info
            self.tabuleiro.push(result.move)

            self.print_game()

            self.lado = "PRETO"
            result = self.black_engine.play(self.tabuleiro, chess.engine.Limit(time=TEMPO_EM_SEGUNDOS))
            self.ultimo_lance = result.move
            self.info = result.info
            self.tabuleiro.push(result.move)

            self.print_game()

            time.sleep(1)

            self.quantidade_de_lances += 1

        resultado = self.tabuleiro.outcome()

        print("Jogo encerrado")
        print(f"{resultado.result()} ({termination_to_string(resultado.termination)})")

        if resultado.winner == chess.WHITE:
            print(f"{self.white} vence")

            if self.white == "capizero_local":
                self.local_score += 1
            else:
                self.release_score += 1
        elif resultado.winner == chess.BLACK:
            print(f"{self.black} vence")

            if self.black == "capizero_local":
                self.local_score += 1
            else:
                self.release_score += 1
        else:
            print("Empate")

            self.local_score += 0.5
            self.release_score += 0.5

        time.sleep(10)

    def end(self):
        self.white_engine.quit()
        self.black_engine.quit()


for jogo_no in range(int(ROUNDS)):
    game = TestGame("capizero_local", "capizero_latest_version", (jogo_no*2) + 1)
    game.play()

    LOCAL_SCORE += game.local_score
    RELEASE_SCORE += game.release_score

    game.end()

    game = TestGame("capizero_latest_version", "capizero_local", (jogo_no*2) + 2)
    game.play()

    LOCAL_SCORE += game.local_score
    RELEASE_SCORE += game.release_score

    game.end()

rating_local = math.log(((10**(int(RELEASE_RATING)/400)) * LOCAL_SCORE) / RELEASE_SCORE, 10) * 400

clear_terminal()

print("Match encerrado")

print(f"Rating local (aprox) = {rating_local}")
print(f"Rating da release = {RELEASE_RATING}")
print(f"Diferença de ratings = {rating_local - int(RELEASE_RATING)}")

