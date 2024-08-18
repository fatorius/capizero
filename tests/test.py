import pip
import sys

from os import execl
from subprocess import check_call


def install(package):
    try:
        pip.main(["install", package])
    except AttributeError:
        check_call([sys.executable, '-m', 'pip', 'install', package])

    execl(sys.executable, sys.executable, *sys.argv)


try:
    import socketio
except ModuleNotFoundError:
    install("python-socketio[client]")

try:
    import chess
    import chess.engine
    import chess.pgn
except ModuleNotFoundError:
    install("chess")

TIME = 2

local_capizero = chess.engine.SimpleEngine.popen_xboard("./capizero")

tabuleiro = chess.Board()


sio = socketio.Client()


@sio.event
def info(data):
    print(f'Info: {data}')


@sio.event
def move(lance):
    print(lance)


@sio.event
def gamestarted(color):
    print(f'Local engine will play as {color}')

    print("GAME STARTED")

    if color == "white":
        print("calc")
        result = local_capizero.play(tabuleiro, chess.engine.Limit(time=TIME))

        tabuleiro.push(result.move)

        print(result.move)

        sio.emit("move", result.move.xboard())


@sio.event
def serverready():
    sio.emit("start")


@sio.event
def connect():
    print("I'm connected!")


@sio.event
def connect_error(data):
    print("The connection failed!")


@sio.event
def disconnect():
    print("I'm disconnected!")


sio.connect('http://localhost:3000')
sio.emit("set time", TIME)


# sio.disconnect()
