#!/bin/bash
if [ `uname` = "Darwin" ]
then
	echo "Building MaMa.osx"
	clang++ -O3 -march=native -integrated-as -DPLATFORM_x64_GNU *.cpp -o ../../bin/MaMa.osx
	strip ../../bin/MaMa.osx
else
	echo "Building MaMa.lnx"
	g++ -O3 -march=native -static -DPLATFORM_x64_GNU *.cpp -o ../../bin/MaMa.lnx
	strip --strip-unneeded ../../bin/MaMa.lnx
fi
