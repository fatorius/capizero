# Initial variables
.DEFAULT_GOAL = default
EPOCH = $(shell date +%s)
VERSION = $(shell cat version.capizero)
CXXFLAGS = -Wall -std=c++11 -DBUILDNO=$(EPOCH) -DCAPIZERO_VERSION=$(shell cat version.capizero)
EXE := capizero
COMP = g++

# Source objects
SRCS = ./src/bitboard.o ./src/init.o \
		./src/update.o ./src/gen.o \
		./src/eval.o ./src/hash.o \
		./src/game.o ./src/search.o \
		./src/interface.o ./src/attacks.o \
		./src/xboard.o ./src/help.o

# Header files
HEADER_FILES = ./src/bitboard.h ./src/init.h \
		./src/update.h ./src/gen.h \
		./src/eval.h ./src/hash.h \
		./src/game.h ./src/search.h \
		./src/interface.h ./src/attacks.h \
		./src/xboard.h ./src/help.h \
		./src/consts.h


# Targets
build: clean ./src/main.o $(SRCS) $(HEADER_FILES)
	@ $(COMP) $(CXXFLAGS) -O3 -o $(EXE) ./src/main.o $(SRCS)
	@ echo "================="
	@ echo "capizero $(VERSION) compilado com sucesso"

debug: clean add_debug_variables ./src/main.o $(SRCS) $(HEADER_FILES)
	@ $(COMP) $(CXXFLAGS) -o $(EXE)_debug ./src/main.o $(SRCS)
	@ echo "================="
	@ echo "capizero $(VERSION) compilado para debug com sucesso"
	@ echo "ATENÇÃO: O BINARIO COMPILADO NÃO CONTEM AS OPTIMIZAÇÕES RECOMENDADAS E DEVE SER USADO SOMENTE PARA TESTES E DEBUG"
	@ echo "Para o uso em jogos normais, use 'make build'"

tests: clean ./src/unit_tests.o ./src/tests.o $(SRCS)
	@ $(COMP) $(CXXFLAGS) -O3 -o capizero_unit_tests ./src/unit_tests.o ./src/tests.o $(SRCS)
	@ echo "================="
	@ echo "testes unitarios compilados com sucesso"


# Other commands
clean:
	@ rm -rf ./src/*.o
	
help:
	@ echo "Para compilar o capizero, você deve usar:"
	@ echo "make [alvo]"
	@ echo ""
	@ echo "Os alvos são: "
	@ echo "======================"
	@ echo "build: compila o capizero com todas as optimizações recomendadas para o uso em jogos"
	@ echo "tests: compila um binário para testes unitários"
	@ echo "debug: compila o capizero sem optimizações e com flags para debug"
	@ echo "======================"
	@ echo "Outros comandos: "
	@ echo "----------------------"
	@ echo "clean: deleta todos os arquivos .o"
	@ echo "help: exibe este menu"
	@ echo "credits: exibe os criadores do capizero"
	@ echo "----------------------"
	@ echo ""

credits: 
	@ echo ""
	@ echo "capizero $(VERSION) é escrito por HugoSouza"
	@ echo ""

default: help credits


# Auxiliary recipes
add_debug_variables:
	@ $(eval CXXFLAGS += -DDEBUG_BUILD -g)

%.o : %.cpp
	@ echo building $@
	@ $(COMP) -c $(CXXFLAGS) $< -o $@
