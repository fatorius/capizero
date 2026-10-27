#!/usr/bin/env python3
"""Smoke test do protocolo UCI com a busca assíncrona (stop, isready, quit).

Uso: python3 tests/uci/smoke.py ./capizero_binario

"""

import queue
import re
import subprocess
import sys
import threading
import time

LANCE_UCI = re.compile(r"^[a-h][1-8][a-h][1-8][nbrq]?$")


class Motor:
    def __init__(self, caminho):
        self.proc = subprocess.Popen(
            [caminho], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, text=True, bufsize=1)
        self.linhas = queue.Queue()
        self.stderr = []
        threading.Thread(target=self._ler, args=(self.proc.stdout, self.linhas.put), daemon=True).start()
        threading.Thread(target=self._ler, args=(self.proc.stderr, self.stderr.append), daemon=True).start()

    @staticmethod
    def _ler(fluxo, guardar):
        for linha in fluxo:
            guardar(linha.rstrip("\n"))

    def enviar(self, *comandos):
        self.proc.stdin.write("".join(c + "\n" for c in comandos))
        self.proc.stdin.flush()

    def esperar(self, prefixo, timeout):
        """Lê até a primeira linha que começa com prefixo; devolve (linha, lidas)."""
        lidas = []
        limite = time.time() + timeout
        while True:
            resta = limite - time.time()
            if resta <= 0:
                raise AssertionError("timeout esperando '%s'; lido: %s" % (prefixo, lidas[-5:]))
            try:
                linha = self.linhas.get(timeout=resta)
            except queue.Empty:
                continue
            lidas.append(linha)
            if linha.startswith(prefixo):
                return linha, lidas

    def nada_em(self, prefixo, segundos):
        """Confere que nenhuma linha com prefixo aparece em `segundos`."""
        limite = time.time() + segundos
        while time.time() < limite:
            try:
                linha = self.linhas.get(timeout=max(0.0, limite - time.time()))
            except queue.Empty:
                return
            if linha.startswith(prefixo):
                raise AssertionError("'%s' inesperado: %s" % (prefixo, linha))

    def encerrar(self, timeout=5):
        if self.proc.poll() is None:
            try:
                self.enviar("quit")
            except BrokenPipeError:
                pass
        self.proc.wait(timeout=timeout)


def lance_do_bestmove(linha):
    partes = linha.split()
    assert len(partes) >= 2 and partes[0] == "bestmove", "bestmove malformado: " + linha
    return partes[1]


def conferir_legal(m, posicao, lance):
    """Aplica o lance numa posição e confere que o motor aceitou."""
    assert LANCE_UCI.match(lance), "lance inválido: " + lance
    m.enviar(posicao + (" " if " moves " in posicao else " moves ") + lance, "isready")
    _, lidas = m.esperar("readyok", 5)
    assert not any("illegal move" in l for l in lidas), "lance ilegal: %s em '%s'" % (lance, posicao)


def iniciar(caminho):
    m = Motor(caminho)
    m.enviar("uci")
    m.esperar("uciok", 5)
    m.enviar("isready")
    m.esperar("readyok", 5)
    return m


# ---------------------------------------------------------------------------
# Casos


def caso_stop_infinite(m):
    """go infinite + stop depois de 1 s: exatamente um bestmove legal."""
    m.enviar("position startpos", "go infinite")
    time.sleep(1.0)
    t0 = time.time()
    m.enviar("stop")
    linha, _ = m.esperar("bestmove", 2)
    assert time.time() - t0 < 0.5, "bestmove demorou %.2f s depois do stop" % (time.time() - t0)
    lance = lance_do_bestmove(linha)
    m.enviar("isready")
    _, lidas = m.esperar("readyok", 2)
    assert not any(l.startswith("bestmove") for l in lidas), "bestmove duplicado"
    conferir_legal(m, "position startpos", lance)


def caso_isready_durante_busca(m):
    """isready durante go infinite: readyok antes do bestmove."""
    m.enviar("position startpos", "go infinite")
    time.sleep(0.2)
    t0 = time.time()
    m.enviar("isready")
    _, lidas = m.esperar("readyok", 1)
    assert time.time() - t0 < 0.2, "readyok demorou %.2f s" % (time.time() - t0)
    assert not any(l.startswith("bestmove") for l in lidas), "bestmove antes do stop"
    m.enviar("stop")
    m.esperar("bestmove", 2)


def caso_stop_imediato(m):
    """stop logo depois do go (antes de a busca começar): bestmove legal, nunca 0000."""
    for _ in range(50):
        m.enviar("position startpos moves e2e4", "go depth 30", "stop")
        linha, _ = m.esperar("bestmove", 2)
        lance = lance_do_bestmove(linha)
        assert lance != "0000", "bestmove 0000 com lances legais"
        conferir_legal(m, "position startpos moves e2e4", lance)


def caso_infinite_termina_sozinho(m):
    """Mate em 1 com go infinite: a busca acaba sozinha, mas o bestmove espera o stop."""
    m.enviar("position fen 6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1", "go infinite")
    m.nada_em("bestmove", 0.5)
    m.enviar("stop")
    linha, _ = m.esperar("bestmove", 2)
    assert lance_do_bestmove(linha) == "a1a8", "esperava a1a8: " + linha


def caso_sem_lances(m):
    """Mate e afogamento na raiz: bestmove 0000."""
    for fen in ("7k/6Q1/6K1/8/8/8/8/8 b - - 0 1",      # mate
                "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1"):     # afogamento
        m.enviar("position fen " + fen, "go depth 5")
        linha, _ = m.esperar("bestmove", 5)
        assert lance_do_bestmove(linha) == "0000", "esperava 0000 em %s: %s" % (fen, linha)


def caso_comandos_durante_busca(m):
    """ucinewgame e setoption Hash esperam a busca acabar; o motor continua respondendo."""
    m.enviar("position startpos", "go depth 9", "ucinewgame", "setoption name Hash value 16", "isready")
    linha, lidas = m.esperar("readyok", 30)
    bestmoves = [l for l in lidas if l.startswith("bestmove")]
    assert len(bestmoves) == 1, "esperava um bestmove antes do readyok: %s" % bestmoves
    m.enviar("setoption name Hash value 128", "isready")
    m.esperar("readyok", 10)


def caso_partida_movetime(m):
    """Até 100 lances com go movetime 20/60, sempre aplicando o bestmove na posição seguinte.

    Abaixo de 50 ms a busca para no primeiro poll (verificar_tempo); com 60 ms
    ela para no meio de uma iteração pelo relógio.
    """
    lances = []
    for i in range(100):
        posicao = "position startpos" + (" moves " + " ".join(lances) if lances else "")
        m.enviar(posicao, "go movetime %d" % (20 if i % 2 else 60))
        linha, lidas = m.esperar("bestmove", 2)
        assert not any("illegal move" in l for l in lidas), "lance ilegal na partida: " + lidas[0]
        lance = lance_do_bestmove(linha)
        if lance == "0000":
            break
        assert LANCE_UCI.match(lance), "lance inválido: " + lance
        lances.append(lance)
    m.enviar("isready")
    _, lidas = m.esperar("readyok", 2)
    assert not any("illegal move" in l for l in lidas), "último lance ilegal"


def caso_quit_durante_busca(caminho):
    """quit durante go infinite: o processo sai em menos de 1 s."""
    m = iniciar(caminho)
    m.enviar("position startpos", "go infinite")
    time.sleep(0.3)
    t0 = time.time()
    m.enviar("quit")
    m.proc.wait(timeout=2)
    assert time.time() - t0 < 1.0, "quit demorou %.2f s" % (time.time() - t0)
    assert m.proc.returncode == 0, "código de saída %d" % m.proc.returncode
    return m


def caso_fim_da_entrada(caminho):
    """stdin fechado durante a busca: o motor imprime o bestmove e sai."""
    m = iniciar(caminho)
    m.enviar("position startpos", "go infinite")
    time.sleep(0.3)
    m.proc.stdin.close()
    m.esperar("bestmove", 2)
    m.proc.wait(timeout=2)
    assert m.proc.returncode == 0, "código de saída %d" % m.proc.returncode
    return m


CASOS_COMPARTILHADOS = [
    caso_stop_infinite,
    caso_isready_durante_busca,
    caso_stop_imediato,
    caso_infinite_termina_sozinho,
    caso_sem_lances,
    caso_comandos_durante_busca,
    caso_partida_movetime,
]

CASOS_PROPRIOS = [
    caso_quit_durante_busca,
    caso_fim_da_entrada,
]


def relatorio_tsan(m):
    return [l for l in m.stderr if "ThreadSanitizer" in l]


def main():
    if len(sys.argv) != 2:
        print("uso: %s <binário do capizero>" % sys.argv[0], file=sys.stderr)
        return 2
    caminho = sys.argv[1]
    falhas = 0

    def rodar(nome, f):
        nonlocal falhas
        t0 = time.time()
        try:
            f()
            print("ok     %-32s %.2f s" % (nome, time.time() - t0))
        except Exception as e:
            falhas += 1
            print("FALHOU %-32s %s" % (nome, e))

    m = iniciar(caminho)
    for caso in CASOS_COMPARTILHADOS:
        rodar(caso.__name__, lambda: caso(m))
    m.encerrar()
    motores = [m]

    for caso in CASOS_PROPRIOS:
        def f(caso=caso):
            motores.append(caso(caminho))
        rodar(caso.__name__, f)

    time.sleep(0.2)
    tsan = [l for mm in motores for l in relatorio_tsan(mm)]
    if tsan:
        falhas += 1
        print("FALHOU ThreadSanitizer: %d relatório(s)" % len(tsan))
        for l in tsan[:5]:
            print("       " + l)

    print("%d falha(s)" % falhas)
    return 1 if falhas else 0


if __name__ == "__main__":
    sys.exit(main())
