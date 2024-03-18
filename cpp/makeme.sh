g++ -I include $(python3 -m pybind11 --includes) -g $(ls src/*.cpp | grep -v 'src/pybindings.cpp') -o a.out -O3 -march=native

