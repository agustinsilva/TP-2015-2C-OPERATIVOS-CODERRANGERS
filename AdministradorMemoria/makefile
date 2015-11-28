CC=gcc
DEPS=src/administradorMemoria.h
RM=rm -f
CFLAGS= -lSockets -lcommons -pthread -g3
#Compilar administrador de Memoria
all: Debug/AdministradorMemoria

Debug/%.o:src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Debug/AdministradorMemoria: DebugDir  Debug/administradorMemoria.o Debug/atencionPedidosCPU.o Debug/busquedas.o Debug/configuracionMemoria.o Debug/gestionMemoria.o Debug/signals.o
	$(CC)  -g -fPIC -Wall -pthread  -fmessage-length=0 -o  Debug/"AdministradorMemoria" Debug/administradorMemoria.o Debug/atencionPedidosCPU.o Debug/busquedas.o Debug/configuracionMemoria.o Debug/gestionMemoria.o Debug/signals.o -lSockets -lcommons

DebugDir:
	mkdir -p Debug

clean:
	$(RM) Debug/administradorMemoria.o
	$(RM) Debug/atencionPedidosCPU.o	
	$(RM) Debug/busquedas.o
	$(RM) Debug/configuracionMemoria.o
	$(RM) Debug/gestionMemoria.o
	$(RM) Debug/signals.o
	$(RM) Debug/AdministradorMemoria.h.gch
	$(RM) Debug/AdministradorMemoria

.PHONY: clean all
