BIN=$(shell basename $$PWD)

$(BIN) : main.cpp
	g++ -ggdb3 -fopenmp $< -o$@  -I/home/micki/phd/code/compiler/spl-transformer/test-suite -I/home/micki/phd/code/compiler/spl-transformer ../../dummy.o



clean :
	rm -f $(BIN)


.PHONY: clean
