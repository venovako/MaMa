#!/bin/bash
if [ `uname` = "Darwin" ]
then
	echo "Building MaMaC.osx"
	clang++ -Ofast -DPLATFORM_x64_GNU *.cpp -o MaMaC.x64 -target x86_64-apple-macos11
	clang++ -Ofast -DPLATFORM_x64_GNU *.cpp -o MaMaC.arm -target arm64-apple-macos11
	lipo -create -output MaMaC.osx MaMaC.x64 MaMaC.arm
	rm -f MaMaC.x64 MaMaC.arm
	mv -f MaMaC.osx ../../bin
	lipo -detailed_info ../../bin/MaMaC.osx
else
	echo "Building MaMaC.lnx"
	g++ -Ofast -static -DPLATFORM_x64_GNU *.cpp -o ../../bin/MaMaC.lnx
	strip --strip-unneeded ../../bin/MaMaC.lnx
fi
