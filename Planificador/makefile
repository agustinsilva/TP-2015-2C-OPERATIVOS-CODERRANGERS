all: planificador

planificador: planificador.o 
	gcc -L../commons  -Wall -g -o job job.o  -lcommons

planificador.o: planificador.c
	gcc -g -c  planificador.c planificador.h

clean:
	rm planificador.o  planificador planificador.h.gch
