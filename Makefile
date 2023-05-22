# Initial variables
.DEFAULT_GOAL = default
EPOCH = $(shell date +%s)
CXXFLAGS = -Wall -std=c++11 -DBUILDNO=$(EPOCH) -DCAPIZERO_VERSION=$(shell cat version.capizero)
EXE := capizero
COMP = g++

# Source objects
SRCS = ./src/bitboard.o ./src/main.o  \
		./src/update.o ./src/gen.o \
		./src/eval.o ./src/hash.o \
		./src/game.o ./src/search.o \
		./src/interface.o ./src/attacks.o \
		./src/xboard.o ./src/help.o

# Recepies
build: clean $(SRCS)
	@ $(COMP) $(CXXFLAGS) -O3 -o $(EXE) $(SRCS)
	@ echo "================="
	@ echo "capizero succesfully built"

debug: clean $(SRCS)
	@ $(COMP) $(CXXFLAGS) -DDEBUG_BUILD -g -o $(EXE)_debug $(SRCS)
	@ echo "================="
	@ echo "capizero succesfully built for debug"

%.o : %.cpp
	@ echo $@
	@ $(COMP) -c $(CXXFLAGS) $< -o $@

clean:
	@ rm -rf ./src/*.o
	
help:
	@ echo "If you want to compile capizero you must use:"
	@ echo "make target"
	@ echo ""
	@ echo "Available targets are: "
	@ echo "======================"
	@ echo "build: build capizero"
	@ echo "clean: delete all .o files"
	@ echo "debug: build capizero with debug symbols and without optmization flags"
	@ echo "help: shows all possible commands"
	@ echo "credits: displays capizero creators"
	@ echo "======================"
	@ echo ""

credits: 
	@ echo ""
	@ echo "capizero is written by HugoSouza"
	@ echo ""

default: help credits