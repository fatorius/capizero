# -----------------------------------------------------
# Variáveis iniciais
.DEFAULT_GOAL = default
EPOCH = $(shell date +%s)
VERSION = $(shell cat version.capizero)
VERSION_WITHOUTQUOTES = $(patsubst '"%"',%, $(VERSION))
CXXFLAGS = -Wall -std=c++11 -O3 -march=native -flto -DBUILDNO=$(EPOCH) -DCAPIZERO_VERSION=$(shell cat version.capizero)
CXXDEBUGFLAGS = -Wall -std=c++11 -DBUILDNO=$(EPOCH) -DCAPIZERO_VERSION=$(shell cat version.capizero) -g -DDEBUG_BUILD
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
		./src/xboard.o ./src/help.o \
		./src/debug.o

# Headers
HEADER_FILES = ./src/bitboard.h ./src/init.h \
		./src/update.h ./src/gen.h \
		./src/eval.h ./src/hash.h \
		./src/game.h ./src/search.h \
		./src/interface.h ./src/attacks.h \
		./src/xboard.h ./src/help.h \
		./src/consts.h ./src/params.h \
		./src/tests.h ./src/values.h \
		./src/stats.h ./src/debug.h ./src/magics.h


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

tests: clean ./src/unit_tests.o ./src/tests.o $(SRCS)
	@ $(COMP) $(CXXFLAGS) -o capi_tests ./src/unit_tests.o ./src/tests.o $(SRCS)
	@ echo "================="
	@ echo "capi_tests compilado com sucesso"

bench: clean ./src/bench_tests.o ./src/bench.o $(SRCS)
	@ $(COMP) $(CXXFLAGS) -o capi_bench ./src/bench_tests.o ./src/bench.o $(SRCS)
	@ echo "================="
	@ echo "capi_bench compilado com sucesso"

magics: ./src/generate_magics.cpp
	@ $(COMP) -c $(CXXFLAGS) ./src/generate_magics.cpp -o ./src/generate_magics.o
	@ $(COMP) -o generate_magics ./src/generate_magics.o 
	@ echo "================="
	@ echo "generate_magics compilado com sucesso"


# -----------------------------------------------------
# Outros comandos
clean:
	@ rm -rf ./src/*.o
	
help:
	@ echo "Para compilar o capizero, você deve usar:"
	@ echo "make [alvo] [opções]"
	@ echo ""
	@ echo "Os alvos são: "
	@ echo "======================"
	@ echo "build: compila o capizero com todas as optimizações recomendadas para o uso em jogos"
	@ echo "tests: compila um binário para testes unitários"
	@ echo "debug: compila o capizero sem optimizações e com flags para debug"
	@ echo "stats: compila um binário para testar a performance da engine no seu computador"
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
