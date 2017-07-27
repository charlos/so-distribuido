#!/bin/bash
if [ -d "esther-deploy" ]; then rm -Rf esther-deploy; fi
mkdir esther-deploy
cd esther-deploy
mkdir shared-library
mkdir memoria
mkdir file-system
mkdir kernel
mkdir cpu
mkdir consola
cd ..


# shared-library
cd ./shared-library/shared-library
make clean
make all
cd ..
cd ..
cp ./shared-library/shared-library/libshared-library.so ./esther-deploy/shared-library
unset LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/home/utnso/esther/esther-deploy/shared-library
echo $LD_LIBRARY_PATH


# memoria
cd ./memoria/src
make clean
make all
cd ..
cd ..
cp ./memoria/src/memoria ./esther-deploy/memoria
cp ./memoria/src/memoria.cfg ./esther-deploy/memoria


# file-system
cd ./file-system/src
make clean
make all
cd ..
cd ..
cp ./file-system/src/file-system ./esther-deploy/file-system
cp ./file-system/src/file-system.cfg ./esther-deploy/file-system


# kernel
cd ./kernel/src
make clean
make all
cd ..
cd ..
cp ./kernel/src/kernel ./esther-deploy/kernel
cp ./kernel/Debug/kernel.cfg ./esther-deploy/kernel


# cpu
cd ./cpu/src
make clean
make all
cd ..
cd ..
cp ./cpu/src/cpu ./esther-deploy/cpu
cp ./cpu/src/cpu.cfg ./esther-deploy/cpu


# consola
cd ./consola/src
make clean
make all
cd ..
cd ..
cp ./consola/src/consola ./esther-deploy/consola
cp ./consola/Debug/consola.cfg ./esther-deploy/consola
