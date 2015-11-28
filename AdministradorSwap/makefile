CC=gcc
DEPS=src/administradorSwap.h
RM=rm -f
CFLAGS= -lSockets -lcommons -g3
#Compilar administrador de swap
all: Debug/AdministradorSwap

Debug/%.o:src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Debug/AdministradorSwap: DebugDir  Debug/administradorSwap.o Debug/conexion.o Debug/configuracionSwap.o Debug/particion.o
	$(CC)  -g -fPIC -Wall  -fmessage-length=0 -o  Debug/"AdministradorSwap" Debug/administradorSwap.o Debug/conexion.o Debug/configuracionSwap.o Debug/particion.o -lSockets -lcommons

DebugDir:
	mkdir -p Debug

clean:
	$(RM) Debug/administradorSwap.o
	$(RM) Debug/conexion.o	
	$(RM) Debug/configuracionSwap.o
	$(RM) Debug/particion.o
	$(RM) Debug/administradorSwap.h.gch
	$(RM) Debug/AdministradorSwap

.PHONY: clean all
