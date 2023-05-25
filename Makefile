# Initial variables
.DEFAULT_GOAL = default
EPOCH = $(shell date +%s)
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
	@ echo "capizero succesfully built"

debug: clean add_debug_variables ./src/main.o $(SRCS) $(HEADER_FILES)
	@ $(COMP) $(CXXFLAGS) -o $(EXE)_debug ./src/main.o $(SRCS)
	@ echo "================="
	@ echo "capizero succesfully built for debug"

tests: clean ./src/unit_tests.o ./src/tests.o $(SRCS)
	@ $(COMP) $(CXXFLAGS) -O3 -o capizero_unit_tests ./src/unit_tests.o ./src/tests.o $(SRCS)
	@ echo "================="
	@ echo "unit tests built succesfully"


# Other commands
clean:
	@ rm -rf ./src/*.o
	
help:
	@ echo "If you want to compile capizero you must use:"
	@ echo "make target"
	@ echo ""
	@ echo "Available targets are: "
	@ echo "======================"
	@ echo "build: build capizero"
	@ echo "tests: build unity tests"
	@ echo "debug: build capizero with debug symbols and without optmization flags"
	@ echo "======================"
	@ echo "Other commands: "
	@ echo "----------------------"
	@ echo "clean: delete all .o files"
	@ echo "help: shows all possible commands"
	@ echo "credits: displays capizero creators"
	@ echo "----------------------"
	@ echo ""

credits: 
	@ echo ""
	@ echo "capizero is written by HugoSouza"
	@ echo ""

default: help credits


# Auxiliary recipes
add_debug_variables:
	@ $(eval CXXFLAGS += -DDEBUG_BUILD -g)

%.o : %.cpp
	@ echo building $@
	@ $(COMP) -c $(CXXFLAGS) $< -o $@
