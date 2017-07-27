#!/bin/bash
if [ -d "esther-deploy" ]; then rm -Rf esther-deploy; fi
mkdir esther-deploy
cd esther-objects
mkdir shared-library
mkdir memoria
mkdir file-system
mkdir kernel
mkdir cpu
mkdir consola
cd


# shared-library
cd ./esther/shared-library/shared-library
make clean
make all
cd
cp ./esther/shared-library/shared-library/libshared-library.so ./esther-objects/shared-library
unset LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/home/utnso/esther-objects/shared-library
echo $LD_LIBRARY_PATH


# memoria
cd ./esther/memoria/src
make clean
make all
cd
cp ./esther/memoria/src/memoria ./esther-objects/memoria
cp ./esther/memoria/src/memoria.cfg ./esther-objects/memoria


# file-system
cd ./esther/file-system/src
make clean
make all
cd
cp ./esther/file-system/src/file-system ./esther-objects/file-system
cp ./esther/file-system/src/file-system.cfg ./esther-objects/file-system


# kernel
cd ./esther/kernel/src
make clean
make all
cd
cp ./esther/kernel/src/kernel ./esther-objects/kernel
cp ./esther/kernel/Debug/kernel.cfg ./esther-objects/kernel


# cpu
cd ./esther/cpu/src
make clean
make all
cd
cp ./esther/cpu/src/cpu ./esther-objects/cpu
cp ./esther/cpu/src/cpu.cfg ./esther-objects/cpu


# consola
cd ./esther/consola/src
make clean
make all
cd
cp ./esther/consola/src/cpu ./esther-objects/cpu
cp ./esther/consola/Debug/cpu.cfg ./esther-objects/cpu
