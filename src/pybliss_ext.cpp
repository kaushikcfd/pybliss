#include <nanobind/nanobind.h>

namespace nb = nanobind;

using namespace nb::literals;

NB_MODULE(pybliss_ext, m) {
    m.doc() = "Wrapper for BLIS-toolkit for graph canonicalization.";
    m.def("add", [](int a, int b) { return a + b; }, "a"_a, "b"_a);
}
