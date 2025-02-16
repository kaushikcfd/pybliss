#include <bliss/graph.hh>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>

namespace nb = nanobind;

using namespace nb::literals;
using namespace bliss;

namespace {
void _perform_sanity_checks_on_perm_array(
    const nb::ndarray<uint32_t, nb::ndim<1>> &ary, size_t reqd_size) {

  // Ensure the array is contiguous and has uint32_t dtype
  if ((ary.stride(0) != 1) || ary.dtype() != nb::dtype<uint32_t>()) {
    throw std::runtime_error("Input array must be a contiguous uint32 array.");
  }
  if (ary.size() != reqd_size) {
    throw std::runtime_error(
        "Shape of permutation array must be equal to number of "
        "vertices in the graph.");
  }
}
} // namespace

NB_MODULE(pybliss_ext, m) {
  m.doc() = "Wrapper for BLISS-toolkit for graph canonicalization.";
  m.def(
      "add", [](int a, int b) { return a + b; }, "a"_a, "b"_a);

  nb::class_<Graph>(m, "Graph")
      .def(nb::init<>())
      .def(
          "set_verbose_level", &Graph::set_verbose_level, "level"_a,
          "Set the verbose output level for the algorithms\n"
          ":arg level: The level of verbose output, 0 means no verbose output.")
      .def(
          "set_verbose_file",
          [](Graph &self, nb::object fp_obj) {
            // Check if the Python object is None (indicating a null FILE*)
            if (fp_obj.is_none()) {
              self.set_verbose_file(nullptr);
            } else {
              // Check if the object has a 'fileno' method (i.e., it's a
              // file-like object)
              if (!nb::hasattr(fp_obj, "fileno")) {
                throw std::runtime_error(
                    "Expected a file-like object with a 'fileno' method");
              }

              // Extract the FILE* from the Python object
              int fd = nb::cast<int>(fp_obj.attr("fileno")());
              FILE *fp =
                  fdopen(fd, "w"); // Open a FILE* from the file descriptor
              if (!fp) {
                throw std::runtime_error(
                    "Failed to open FILE* from file descriptor");
              }
              self.set_verbose_file(fp);
            }
          },
          "fp"_a,
          "Set the file stream for verbose output.\n\n"
          ":param file_obj: The file object to write the output to. If None, "
          "writing to the file is disabled.")
      .def("add_vertex", &Graph::add_vertex, "color"_a = 0,
           "Add a new vertex with color *color* and return its new index.")
      .def("add_edge", &Graph::add_edge, "source"_a, "target"_a,
           "Add an edge from *source* to *target*.")
      .def("get_color", &Graph::get_color, "v"_a,
           "Returns the color of the vertex *v*")
      .def("change_color", &Graph::change_color, "v"_a, "c"_a,
           "Change the color of vertex *v* to *c*.")
      // TODO: Think if this should not be exposed.
      .def("set_failure_recording", &Graph::set_failure_recording, "active"_a,
           "Activate / deactivate failure recording\n\n"
           ":arg active:If true, activate failure recording, deactivate "
           "otherwise.")
      .def("set_component_recursion", &Graph::set_component_recursion,
           "active"_a,
           "Activate/deactivate component recursion. The choice affects the "
           "computed canonical labelings; therefore, if you want to compare "
           "whether two graphs are isomorphic by computing and comparing (for "
           "equality) their canonical versions, be sure to use the same choice "
           "for both graphs. May not be called during the search, i.e. from an "
           "automorphism reporting hook function.\n\n"
           ":arg active:  If true, activate component recursion, deactivate "
           "otherwise.")
      .def_prop_ro(
          "nvertices", [](Graph &self) { return self.get_nof_vertices(); },
          "Return the number of vertices in the graph.")
      .def(
          "permute",
          [](Graph &self, const nb::ndarray<uint32_t, nb::ndim<1>> &ary) {
            _perform_sanity_checks_on_perm_array(ary, self.get_nof_vertices());
            return self.permute((uint32_t *)ary.data());
          },
          "perm"_a,
          " Return a new graph that is the result of applying the permutation "
          "*perm* to this graph. This graph is not modified. *perm* must "
          "contain N=this.get_nof_vertices() elements and be a bijection on "
          "{0,1,...,N-1}, otherwise the result is undefined.")
      .def(
          "is_automorphism",
          [](Graph &self, const nb::ndarray<uint32_t, nb::ndim<1>> &ary) {
            _perform_sanity_checks_on_perm_array(ary, self.get_nof_vertices());
            return self.is_automorphism((uint32_t *)ary.data());
          },
          "perm"_a,
          " Return true only if *perm* is an automorphism of this graph."
          " *perm* must contain N=this.get_nof_vertices() elements and be a"
          " bijection on {0,1,...,N-1}, otherwise the result is undefined.");
}
