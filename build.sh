mkdir build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -S . -B ./build
cd build
make -j