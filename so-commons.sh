#!/bin/bash
if [ -d "so-commons-library" ]; then rm -Rf so-commons-library; fi
mkdir so-commons-library
git clone https://dromero-7854:ASDzxc7854@github.com/sisoputnfrba/so-commons-library.git ./so-commons-library

cd ./so-commons-library
make
sudo make install

cd
rm -Rf so-commons-library;
