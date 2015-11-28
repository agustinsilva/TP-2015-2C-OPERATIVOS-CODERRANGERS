CC=gcc
DEPS=src/planificador.h
RM=rm -f
CFLAGS= -lSockets -lcommons -pthread -g3
#Compilar planificador
all: Debug/Planificador

Debug/%.o:src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Debug/Planificador: DebugDir  Debug/configuracionPlanificador.o Debug/consola.o Debug/planificador.o Debug/servidor.o
	$(CC)  -g -fPIC -Wall -pthread  -fmessage-length=0 -o  Debug/"Planificador" Debug/configuracionPlanificador.o Debug/consola.o Debug/planificador.o Debug/servidor.o -lSockets -lcommons

DebugDir:
	mkdir -p Debug

clean:
	$(RM) Debug/configuracionPlanificador.o
	$(RM) Debug/consola.o	
	$(RM) Debug/planificador.o
	$(RM) Debug/servidor.o
	$(RM) Debug/Planificador.h.gch
	$(RM) Debug/Planificador

.PHONY: clean all
