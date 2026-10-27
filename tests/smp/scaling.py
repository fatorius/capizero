#!/usr/bin/env python3
"""Escalonamento do Lazy SMP: tempo até a profundidade e NPS por número de threads.

Uso: python3 tests/smp/scaling.py ./capizero_binario [profundidade] [threads...]
     (padrão: profundidade 14, threads 1 2 4)

Para cada número de threads, roda "go depth N" em cada FEN do bench (TT limpa
entre posições) e soma tempo e nós. Esperado: NPS total quase linear e tempo
até a profundidade caindo. Se o tempo não cair, as threads auxiliares não
estão ajudando a principal (falta diversificação).

Rodar com a máquina livre: com outros processos disputando os cores, os
números não querem dizer nada.
"""

import os
import re
import subprocess
import sys

RAIZ = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
INFO = re.compile(r"^info depth (\d+) .*time (\d+) nodes (\d+)")


def fens_do_bench():
    with open(os.path.join(RAIZ, "src", "bench_fens.h")) as f:
        return re.findall(r'"([^"]+/[^"]+)"', f.read())


def medir(binario, threads, profundidade, fens):
    p = subprocess.Popen([binario], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1)

    def enviar(cmd):
        p.stdin.write(cmd + "\n")
        p.stdin.flush()

    def esperar(prefixo):
        ultimo = None
        for linha in p.stdout:
            if INFO.match(linha):
                ultimo = linha
            if linha.startswith(prefixo):
                return ultimo
        raise RuntimeError("motor fechou antes de '%s'" % prefixo)

    enviar("uci")
    esperar("uciok")
    enviar("setoption name Threads value %d" % threads)
    enviar("isready")
    esperar("readyok")

    tempo = nos = 0
    for fen in fens:
        enviar("ucinewgame")
        enviar("position fen " + fen)
        enviar("go depth %d" % profundidade)
        info = esperar("bestmove")
        m = INFO.match(info or "")
        if not m or int(m.group(1)) != profundidade:
            raise RuntimeError("sem info depth %d em %s" % (profundidade, fen))
        tempo += int(m.group(2))
        nos += int(m.group(3))

    enviar("quit")
    p.wait()
    return tempo, nos


def main():
    if len(sys.argv) < 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2
    binario = sys.argv[1]
    profundidade = int(sys.argv[2]) if len(sys.argv) > 2 else 14
    lista = [int(t) for t in sys.argv[3:]] or [1, 2, 4]
    fens = fens_do_bench()

    print("%d FENs, go depth %d" % (len(fens), profundidade))
    print("%7s %10s %14s %12s %10s %8s" % ("threads", "tempo ms", "nós", "NPS", "NPS/thr", "speedup"))
    base = None
    for t in lista:
        tempo, nos = medir(binario, t, profundidade, fens)
        nps = nos * 1000 // max(tempo, 1)
        if base is None:
            base = tempo
        print("%7d %10d %14d %12d %10d %7.2fx" % (t, tempo, nos, nps, nps // t, base / max(tempo, 1)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
