FOLDER=$(pwd)
#cd build/mgport || exit
#ctest -T memcheck
#cd "$FOLDER" || exit

#compile
#cmake .. -DCMAKE_BUILD_TYPE=Debug
#make -j4

cd build/fgport/test || exit
./hello_test
