#!/bin/bash
if [ `uname` = "Darwin" ]
then
	echo "Building MaMaC.osx"
	clang++ -O3 -march=native -integrated-as -DPLATFORM_x64_GNU *.cpp -o ../../bin/MaMaC.osx
	strip ../../bin/MaMaC.osx
else
	echo "Building MaMaC.lnx"
	g++ -O3 -march=native -static -DPLATFORM_x64_GNU *.cpp -o ../../bin/MaMaC.lnx
	strip --strip-unneeded ../../bin/MaMaC.lnx
fi
