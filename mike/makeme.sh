g++ -I include $(python3 -m pybind11 --includes) -g  src/*.cpp -o a.out -O3 -march=native -std=c++17
#cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

