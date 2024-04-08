g++ -I include $(python3 -m pybind11 --includes) -g  src/*.cpp -o a.out -O3 -march=native

