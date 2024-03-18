
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "bench.h"
namespace Poker {

  PYBIND11_MODULE(poker, m) {
      m.def("MCGames", &monteCarloGames, "Monte Carlo Games",
          pybind11::arg("N"), pybind11::arg("aiList"));
  }
}