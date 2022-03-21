assembler: assembler.o
	gcc -ansi -Wall -pedantic -g assembler.o -o assembler -lm
assembler.o: assembler.c
	gcc -c -ansi -pedantic -Wall assembler.c -o assembler.o -lm
                   
