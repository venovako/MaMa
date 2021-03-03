#!/bin/bash
if [ `uname` = "Darwin" ]
then
	echo "Building MaMa.osx"
	clang++ -O3 -DPLATFORM_x64_GNU *.cpp -o MaMa.x64 -target x86_64-apple-macos11
	clang++ -O3 -DPLATFORM_x64_GNU *.cpp -o MaMa.arm -target arm64-apple-macos11
	lipo -create -output MaMa.osx MaMa.x64 MaMa.arm
	rm -f MaMa.x64 MaMa.arm
	mv -f MaMa.osx ../../bin
	lipo -detailed_info ../../bin/MaMa.osx
else
	echo "Building MaMa.lnx"
	g++ -O3 -static -DPLATFORM_x64_GNU *.cpp -o ../../bin/MaMa.lnx
	strip --strip-unneeded ../../bin/MaMa.lnx
fi
