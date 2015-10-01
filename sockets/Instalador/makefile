all: compilar

compilar:
	 gcc -shared -fPIC -o "libSockets.so" ../sockets/socket.c ../sockets/socket.h

install: compilar
	cp -u libSockets.so /usr/lib
	cp -u ../sockets/socket.h /usr/include
	make clean

uninstall:
	rm -f /usr/lib/libSockets.so
	rm -rf /usr/include/socket.h

clean:
	rm -f libSockets.so

.PHONY: all install uninstall compilar
