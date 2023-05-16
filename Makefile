CXXFLAGS = -Wall -std=c++11
EXE := capizero
COMP = g++

SRCS = ./src/bitboard.o ./src/main.o  \
		./src/update.o ./src/gen.o \
		./src/eval.o ./src/hash.o

build: $(SRCS)
	@ $(COMP) $(CXXFLAGS) -o $(EXE) $(SRCS)
	@ echo "================="
	@ echo "capizero succesfully built"

%.o : %.cpp
	@ echo $@
	@ $(COMP) -c $(CXXFLAGS) $< -o $@

clean:
	@ rm -rf ./src/*.o
	