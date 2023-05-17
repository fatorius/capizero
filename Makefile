# Initial variables
.DEFAULT_GOAL = default
CXXFLAGS = -Wall -std=c++11
EXE := capizero
COMP = g++
EPOCH = $(shell date +%s)

# Source objects
SRCS = ./src/bitboard.o ./src/main.o  \
		./src/update.o ./src/gen.o \
		./src/eval.o ./src/hash.o \
		./src/game.o ./src/search.o \
		./src/xboard.o ./src/attacks.o


# Aditional flag configuration
ifeq ($(DEBUG), TRUE)
CXXFLAGS += -g
EXE := $(EXE)_debug
endif

# Set compilation variables
CXXFLAGS += -DBUILDNO=$(EPOCH) -DCAPIZERO_VERSION=$(shell cat version.capizero)


# Recepies
build: $(SRCS)
	@ $(COMP) $(CXXFLAGS) -o $(EXE) $(SRCS)
	@ echo "================="
	@ echo "capizero succesfully built"

%.o : %.cpp
	@ echo $@
	@ $(COMP) -c $(CXXFLAGS) $< -o $@

clean:
	@ rm -rf ./src/*.o
	
help:
	@ echo "If you want to compile capizero you must use:"
	@ echo "make target [params]"
	@ echo ""
	@ echo "Available targets are: "
	@ echo "======================"
	@ echo "build: build capizero"
	@ echo "clean: delete all .o files"
	@ echo "help: shows all possible commands"
	@ echo "credits: displays capizero creators"
	@ echo "======================"
	@ echo ""
	@ echo "Usable parameters are: "
	@ echo "======================"
	@ echo "DEBUG: compiles with debug flags that can be used in a C++ Debugger tool"
	@ echo "All params default value is FALSE"
	@ echo "======================"
	@ echo "Example: make build DEBUG=TRUE"
	@ echo ""

credits: 
	@ echo ""
	@ echo "capizero is written by HugoSouza"
	@ echo ""

default: help credits