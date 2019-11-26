SRC = main.cc scanner.cc parser.cc codeGen.cc
HEADERS = scanner.h parser.h codeGen.h
PROG = comp

all: $(PROG)

comp: $(SRC) $(HEADERS)
	g++ -g --std=c++11 $(SRC) -o $(PROG)
    
clean:
	rm $(PROG) *.asm
