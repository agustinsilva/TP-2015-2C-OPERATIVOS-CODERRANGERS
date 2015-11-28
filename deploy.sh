#!/bin/bash
cd ..
git clone https://github.com/sisoputnfrba/so-commons-library.git
cd so-commons-library
sudo make install
cd ..
cd tp-2015-2c-coderrangers/sockets/Instalador
sudo make install
cd ..
cd ..
sudo make all
