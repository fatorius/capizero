# -----------------------------------------------------
# Variáveis iniciais
.DEFAULT_GOAL = default
EPOCH = $(shell date +%s)
VERSION = $(shell cat version.capizero)
VERSION_WITHOUTQUOTES = $(patsubst '"%"',%, $(VERSION))
CXXFLAGS = -Wall -std=c++11 -I./src -O3 -march=native -flto -DBUILDNO=$(EPOCH) -DCAPIZERO_VERSION=$(shell cat version.capizero)
CXXDEBUGFLAGS = -Wall -std=c++11 -I./src -DBUILDNO=$(EPOCH) -DCAPIZERO_VERSION=$(shell cat version.capizero) -g -DDEBUG_BUILD
EXE := $(NAME)
COMP = g++


# -----------------------------------------------------
# Configurações

ifeq ($(NAME),)
EXE := capizero_$(VERSION_WITHOUTQUOTES)
endif

# - Compilador
ifeq ($(COMP),gcc)
CXXFLAGS += -DGNUC
else ifeq ($(COMP),g++)
CXXFLAGS += -DGNUC
else ifeq ($(COMP),clang)
CXXFLAGS += -DMSVC
endif

# - PEXT
ifeq ($(PEXT),false)
CXXFLAGS += -DNOT_USE_PEXT
else ifeq ($(PEXT),FALSE)
CXXFLAGS += -DNOT_USE_PEXT
endif

# -----------------------------------------------------
# Objs
SRCS = ./src/bitboard.o ./src/init.o \
		./src/update.o ./src/gen.o \
		./src/eval.o ./src/hash.o \
		./src/game.o ./src/search.o \
		./src/interface.o ./src/attacks.o \
		./src/xboard.o ./src/uci.o \
		./src/bench.o ./src/help.o \
		./src/debug.o

# Headers
HEADER_FILES = ./src/bitboard.h ./src/init.h \
		./src/update.h ./src/gen.h \
		./src/eval.h ./src/hash.h \
		./src/game.h ./src/search.h \
		./src/interface.h ./src/attacks.h \
		./src/xboard.h ./src/uci.h \
		./src/help.h ./src/consts.h \
		./src/params.h \
		./src/values.h ./src/bench.h \
		./src/bench_fens.h ./src/debug.h \
		./src/magics.h


# -----------------------------------------------------
# Targets
build: clean ./src/main.o $(SRCS) $(HEADER_FILES)
	@ $(COMP) $(CXXFLAGS) -o $(EXE) ./src/main.o $(SRCS)
	@ echo "================="
	@ echo "$(EXE) compilado com sucesso"

debug: clean add_debug_variables ./src/main.o $(SRCS) $(HEADER_FILES)
	@ $(COMP) $(CXXFLAGS) -o capi_debug ./src/main.o $(SRCS)
	@ echo "================="
	@ echo "capi_debug compilado com sucesso"

tests: clean ./tests/unit/unit_tests.o ./tests/unit/tests.o $(SRCS)
	@ $(COMP) $(CXXFLAGS) -o capi_tests ./tests/unit/unit_tests.o ./tests/unit/tests.o $(SRCS)
	@ echo "================="
	@ echo "capi_tests compilado com sucesso"

bench: clean ./tests/bench/bench_tests.o $(SRCS)
	@ $(COMP) $(CXXFLAGS) -o capi_bench ./tests/bench/bench_tests.o $(SRCS)
	@ echo "================="
	@ echo "capi_bench compilado com sucesso"

# Texel-style eval tuner. Reuses the engine's eval/board/state code; tuner.cpp
# adds the dataset loader, loss function, and (eventually) coordinate-descent
# tuning loop. Runs as: ./capi_tuner <dataset.txt>
tuner: clean ./tools/tuning/tuner.o $(SRCS)
	@ $(COMP) $(CXXFLAGS) -o capi_tuner ./tools/tuning/tuner.o $(SRCS)
	@ echo "================="
	@ echo "capi_tuner compilado com sucesso"

# Gradient-based Texel tuner (sparse features + Adam). See tools/tuning/tuner_adam.cpp.
# Runs as: ./capi_tuner_adam <dataset.txt> [options]
tuner_adam: clean ./tools/tuning/tuner_adam.o $(SRCS)
	@ $(COMP) $(CXXFLAGS) -pthread -o capi_tuner_adam ./tools/tuning/tuner_adam.o $(SRCS)
	@ echo "================="
	@ echo "capi_tuner_adam compilado com sucesso"

# Self-play data collector — Phase A. Plays engine-vs-engine games, samples
# quiet positions during play, writes them out labeled with the game's WDL
# result. Output feeds capi_resolve (Phase B) and then capi_tuner.
selfplay: clean ./tools/datagen/selfplay.o ./tools/datagen/fen_serializer.o $(SRCS)
	@ $(COMP) $(CXXFLAGS) -o capi_selfplay ./tools/datagen/selfplay.o ./tools/datagen/fen_serializer.o $(SRCS)
	@ echo "================="
	@ echo "capi_selfplay compilado com sucesso"

# PV-resolver — Phase B. For each input position, runs a high-depth search,
# walks the PV through the TT, and emits the leaf position with the original
# game's WDL. Produces the final dataset for capi_tuner.
resolve: clean ./tools/datagen/resolve.o ./tools/datagen/fen_serializer.o $(SRCS)
	@ $(COMP) $(CXXFLAGS) -o capi_resolve ./tools/datagen/resolve.o ./tools/datagen/fen_serializer.o $(SRCS)
	@ echo "================="
	@ echo "capi_resolve compilado com sucesso"

magics: ./tools/magics/generate_magics.cpp
	@ $(COMP) -c $(CXXFLAGS) ./tools/magics/generate_magics.cpp -o ./tools/magics/generate_magics.o
	@ $(COMP) -o generate_magics ./tools/magics/generate_magics.o 
	@ echo "================="
	@ echo "generate_magics compilado com sucesso"


# -----------------------------------------------------
# Outros comandos
clean:
	@ rm -rf ./src/*.o ./tools/*/*.o ./tests/*/*.o
	
help:
	@ echo "Para compilar o capizero, você deve usar:"
	@ echo "make [alvo] [opções]"
	@ echo ""
	@ echo "Os alvos são: "
	@ echo "======================"
	@ echo "build: compila o capizero com todas as optimizações recomendadas para o uso em jogos"
	@ echo "tests: compila um binário para testes unitários"
	@ echo "debug: compila o capizero sem optimizações e com flags para debug"
	@ echo "bench: compila um binário para testar a performance da engine no seu computador"
	@ echo "tuner: compila o Texel tuner por coordinate descent (tools/tuning)"
	@ echo "tuner_adam: compila o Texel tuner por gradiente/Adam (tools/tuning)"
	@ echo "selfplay: compila o coletor de partidas de self-play (tools/datagen)"
	@ echo "resolve: compila o resolvedor de PV do dataset (tools/datagen)"
	@ echo "magics: compila o gerador de números mágicos (tools/magics)"
	@ echo "======================"
	@ echo "As opções: "
	@ echo "NAME = string: define o nome do binário"
	@ echo "COMP = string: define o compilador (padrão=g++)"
	@ echo "PEXT = [true/false]: define se a engine usará bitboards PEXT (padrão=true) - recomendado desativar para CPUs antigas ou com PEXT lento"
	@ echo ""
	@ echo "Outros comandos: "
	@ echo "----------------------"
	@ echo "clean: deleta todos os arquivos .o"
	@ echo "help: exibe este menu"
	@ echo "credits: exibe os criadores do capizero"
	@ echo "----------------------"
	@ echo ""

credits: 
	@ echo ""
	@ echo "capizero $(VERSION_WITHOUTQUOTES) é escrito por HugoSouza"
	@ echo ""

default: help credits


# -----------------------------------------------------
# Outras receitas
add_debug_variables:
	@ $(eval CXXFLAGS = $(CXXDEBUGFLAGS))

%.o : %.cpp
	@ echo building $@
	@ $(COMP) -c $(CXXFLAGS) $< -o $@
