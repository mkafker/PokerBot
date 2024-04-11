#g++ -I include -g src/*.cpp -o a.out -O3 -march=native
g++  -shared -std=c++20 -fPIC $(python3 -m pybind11 --includes) \
  -I include -g src/*.cpp -O3 -march=native -o poker$(python3-config --extension-suffix)

