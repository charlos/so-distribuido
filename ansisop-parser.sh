#!/bin/bash
if [ -d "ansisop-parser" ]; then rm -Rf ansisop-parser; fi
mkdir ansisop-parser
git clone https://dromero-7854:ASDzxc7854@github.com/sisoputnfrba/ansisop-parser.git ./ansisop-parser

cd ./ansisop-parser/parser
make all
sudo make install

cd ..
cd ..
