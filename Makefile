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

ifeq ($(COMP),gcc)
CXXFLAGS += -DGNU
else ifeq ($(COMP),g++)
CXXFLAGS += -DGNU
else ifeq ($(COMP),clang)
CXXFLAGS += -DMSVC
endif


ifeq ($(PEXT),true)
CXXFLAGS += -DUSEPEXT
else ifeq ($(PEXT),TRUE)
CXXFLAGS += -DUSEPEXT
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
build: clean add_build_variables $(SRCS) $(HEADER_FILES)
	@ $(COMP) $(CXXFLAGS) -o $(EXE) $(SRCS)
	@ echo "================="
	@ echo "$(EXE) compilado com sucesso"

debug: clean add_debug_variables $(SRCS) $(HEADER_FILES)
	@ $(COMP) $(CXXFLAGS) -o $(EXE) $(SRCS)
	@ echo "================="
	@ echo "ATENÇÃO: O BINARIO COMPILADO NÃO CONTEM AS OPTIMIZAÇÕES RECOMENDADAS E DEVE SER USADO SOMENTE PARA TESTES E DEBUG"
	@ echo "Para o uso em jogos normais, use 'make build'"
	@ echo "$(EXE) compilado com sucesso"

tests: clean add_tests_variables $(SRCS) $(HEADER_FILES)
	@ $(COMP) $(CXXFLAGS) -o $(EXE) $(SRCS)
	@ echo "================="
	@ echo "$(EXE) compilado com sucesso"

stats: clean add_stats_variables $(SRCS) $(HEADER_FILES)
	@ $(COMP) $(CXXFLAGS) -o $(EXE) $(SRCS)
	@ echo "================="
	@ echo "$(EXE) compilado com sucesso"



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
	@ echo "PEXT = [true/false]: define se a engine usará bitboards PEXT (experimental) (padrão=false)"
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
# Outras funções
add_build_variables: ./src/main.o
	@ $(eval SRCS += ./src/main.o)

add_debug_variables: ./src/main.o
	@ $(eval SRCS += ./src/main.o)
	@ $(eval EXE = $(EXE)_debug)
	@ $(eval CXXFLAGS = $(CXXDEBUGFLAGS))

add_stats_variables: ./src/stats_tests.o ./src/stats.o
	@ $(eval EXE = $(EXE)_stats)
	@ $(eval SRCS += ./src/stats_tests.o ./src/stats.o)

add_tests_variables: ./src/unit_tests.o ./src/tests.o
	@ $(eval EXE = $(EXE)_tests)
	@ $(eval SRCS += ./src/unit_tests.o ./src/tests.o)



%.o : %.cpp
	@ echo building $@
	@ $(COMP) -c $(CXXFLAGS) $< -o $@
