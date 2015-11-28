all: 
	-cd AdministradorMemoria && $(MAKE) all
	-cd AdministradorSwap && $(MAKE) all
	-cd CPU && $(MAKE) all
	-cd Planificador && $(MAKE) all

clean:
	-cd AdministradorMemoria && $(MAKE) clean
	-cd AdministradorSwap && $(MAKE) clean
	-cd CPU && $(MAKE) clean
	-cd Planificador && $(MAKE) clean
