#!/bin/bash
unzip $1.zip
cd $1
cd codebase
cd rbf
make clean
make
./rbftest1
./rbftest2
./rbftest3
./rbftest4
./rbftest5
./rbftest6
./rbftest7
./rbftest8
./rbftest8b
./rbftest9
./rbftest10
./rbftest11
./rbftest12
