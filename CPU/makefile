CC=gcc
DEPS=src/cpu.h
RM=rm -f
CFLAGS= -lSockets -lcommons -pthread
#Compilar cpu
all: Debug/Cpu

Debug/%.o:src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Debug/Cpu: DebugDir  Debug/Conexion.o Debug/configuracionCpu.o Debug/cpu.o Debug/GestorTiempos.o
	$(CC)  -g -fPIC -Wall -pthread  -fmessage-length=0 -o  Debug/"Cpu" Debug/Conexion.o Debug/configuracionCpu.o Debug/cpu.o Debug/GestorTiempos.o  -lSockets -lcommons

DebugDir:
	mkdir -p Debug

clean:
	$(RM) Debug/Conexion.o
	$(RM) Debug/configuracionCpu.o	
	$(RM) Debug/cpu.o
	$(RM) Debug/GestorTiempos.o
	$(RM) Debug/Cpu.h.gch
	$(RM) Debug/Cpu

.PHONY: clean all
