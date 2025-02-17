#include <bliss/bignum.hh>
#include <bliss/graph.hh>
#include <bliss/stats.hh>
#include <memory>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>

namespace nb = nanobind;

using namespace nb::literals;
using namespace bliss;

namespace {
void perform_sanity_checks_on_perm_array(
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

FILE *get_fp_from_writeable_pyobj(nb::object file_obj) {
  // Check if the object is None (indicating no file)
  if (file_obj.is_none()) {
    throw std::runtime_error("File object is None");
  }

  // Check if the object has a 'fileno' method
  if (!nb::hasattr(file_obj, "fileno")) {
    throw std::runtime_error(
        "Expected a file-like object with a 'fileno' method");
  }

  // Check if the object is writable
  if (!nb::hasattr(file_obj, "write")) {
    throw std::runtime_error(
        "File object is not writable (missing 'write' method)");
  }
  return nb::cast<FILE *>(file_obj.attr("fileno")());
}

FILE *get_fp_from_readable_pyobj(nb::object file_obj) {
  // Check if the object is None (indicating no file)
  if (file_obj.is_none()) {
    throw std::runtime_error("File object is None");
  }

  // Check if the object has a 'fileno' method
  if (!nb::hasattr(file_obj, "fileno")) {
    throw std::runtime_error(
        "Expected a file-like object with a 'fileno' method");
  }

  // Check if the object is writable
  if (!nb::hasattr(file_obj, "read")) {
    throw std::runtime_error(
        "File object is not readable (missing 'read' method)");
  }
  return nb::cast<FILE *>(file_obj.attr("fileno")());
}

std::string
capture_string_written_to_file(std::function<void(FILE *)> file_writer) {
  // unique_ptr to ensure FILE* is properly closed
  std::unique_ptr<FILE, decltype(&fclose)> fp(tmpfile(), fclose);
  if (!fp) {
    throw std::runtime_error("Failed to create temporary file.");
  }

  file_writer(fp.get());
  fflush(fp.get()); // Ensure all output is written

  // Get file size
  if (fseek(fp.get(), 0, SEEK_END) != 0) {
    throw std::runtime_error("Failed to seek to end of file.");
  }
  long size = ftell(fp.get());
  if (size == -1L) {
    throw std::runtime_error("Failed to determine file size.");
  }
  rewind(fp.get()); // Go back to the beginning

  // Read the content into a string
  std::string output(size, '\0');
  fread(&output[0], 1, size, fp.get());

  return output;
}

} // namespace

NB_MODULE(pybliss_ext, m) {
  m.doc() = "Wrapper for BLISS-toolkit for graph canonicalization.";
  m.def(
      "add", [](int a, int b) { return a + b; }, "a"_a, "b"_a);

  // {{{ BigNum

  nb::class_<BigNum>(m, "BigNum",
                     "A wrapper class for non-negative big integers.")
      .def(nb::init<>())
      .def("assign", &BigNum::assign)
      .def("multiply", &BigNum::multiply)
      .def("print_to_file",
           [](Stats &self, nb::object fp_obj) {
             FILE *fp = get_fp_from_writeable_pyobj(fp_obj);
             self.print(fp);
           })
      .def("__str__",
           [](Stats &self) {
             return capture_string_written_to_file(
                 [&](FILE *fp) { self.print(fp); });
           }
           // FIXME: Add a to_python method.
      );

  // }}}

  // {{{ Stats

  nb::class_<Stats>(m, "Stats",
                    "Records statistics returned by the search algorithms.")
      .def(nb::init<>())
      .def("print_to_file",
           [](Stats &self, nb::object fp_obj) {
             FILE *fp = get_fp_from_writeable_pyobj(fp_obj);
             self.print(fp);
           })
      .def_prop_ro(
          "group_size", [](Stats &self) { return self.get_group_size(); },
          "The size of the automorphism group.")
      .def_prop_ro(
          "group_size_approx",
          [](Stats &self) { return self.get_group_size_approx(); },
          "An approximation (due to possible overflows/rounding errors) of the"
          " size of the automorphism group")
      .def_prop_ro(
          "n_nodes", [](Stats &self) { return self.get_nof_nodes(); },
          "Number of nodes in the search tree.")
      .def_prop_ro(
          "n_leaf_nodes", [](Stats &self) { return self.get_nof_leaf_nodes(); },
          "Number of leaf nodes in the search tree.")
      .def_prop_ro(
          "n_bad_nodes", [](Stats &self) { return self.get_nof_bad_nodes(); },
          "Number of bad nodes in the search tree.")
      .def_prop_ro(
          "n_canupdates", [](Stats &self) { return self.get_nof_canupdates(); },
          "Number of canonical representative nodes.")
      .def_prop_ro(
          "n_generators", [](Stats &self) { return self.get_nof_generators(); },
          "Number of generator permutations.")
      .def_prop_ro(
          "max_level", [](Stats &self) { return self.get_max_level(); },
          "The maximal depth of the search tree.")
      .def("__str__", [](Stats &self) {
        return capture_string_written_to_file(
            [&](FILE *fp) { self.print(fp); });
      });

  // }}}

  // {{{ Graph

  nb::class_<Graph>(m, "Graph",
                    "Undirected vertex-colored graph.\n\n"
                    "See :class:`DiGraph` for a directed variant.")
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
              FILE *fp = get_fp_from_writeable_pyobj(fp_obj);
              self.set_verbose_file(fp);
            }
          },
          "fp"_a,
          "Set the file stream for verbose output.\n\n"
          ":param file_obj: The file object to write the output to. If None, "
          "writing to the file is disabled.")
      .def("add_vertex", &Graph::add_vertex, "color"_a = 0,
           "Add a new vertex with color *color* and return its new index.")
      .def("add_edge", &Graph::add_edge, "v1"_a, "v1"_a,
           "Add an edge between *v1* and *v2*.")
      .def("get_color", &Graph::get_color, "v"_a,
           "Returns the color of the vertex *v*")
      .def("change_color", &Graph::change_color, "v"_a, "c"_a,
           "Change the color of vertex *v* to *c*.")
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
            perform_sanity_checks_on_perm_array(ary, self.get_nof_vertices());
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
            perform_sanity_checks_on_perm_array(ary, self.get_nof_vertices());
            return self.is_automorphism((uint32_t *)ary.data());
          },
          "perm"_a,
          " Return true only if *perm* is an automorphism of this graph."
          " *perm* must contain N=this.get_nof_vertices() elements and be a"
          " bijection on {0,1,...,N-1}, otherwise the result is undefined.")
      .def(
          "find_automorphisms",
          [](Graph &self, Stats &stats,
             const std::function<void(
                 int, nb::ndarray<nb::ro, uint32_t, nb::ndim<1>, nb::numpy,
                                  nb::c_contig>)> &py_report = nullptr,
             const std::function<bool()> &py_terminate = nullptr) {
            std::function<void(unsigned int, const unsigned int *)> cpp_report =
                nullptr;
            std::function<bool()> cpp_terminate = nullptr;

            if (py_report) {
              cpp_report = [&](unsigned int n, const unsigned int *aut) {
                auto np_aut =
                    nb::ndarray<nb::ro, uint32_t, nb::ndim<1>, nb::numpy,
                                nb::c_contig>(aut, {self.get_nof_vertices()});
                py_report(n, np_aut);
              };
            }

            if (py_terminate) {
              cpp_terminate = [&]() { return py_terminate(); };
            }

            self.find_automorphisms(stats, cpp_report, cpp_terminate);
          },
          "stats"_a, "report"_a = nb::none(), "terminate"_a = nb::none(),
          "Find a set of generators for the automorphism group of the graph. "
          "The function *report* (if not None) is called each time a new "
          "generator for the automorphism group is found. The first argument "
          "*n* for the function is the length of the automorphism (equal to "
          "get_nof_vertices()), and the second argument *aut* is the "
          "automorphism (a bijection on {0,...,nvertices-1}). *aut* is a "
          "read-only :class:`numpy.ndarray`. Do not call any member functions "
          "from the *report* function.\n\n"

          "The search statistics are copied in *stats*.\n\n"

          "If the *terminate* function argument is given, it is called in each "
          "search tree node: if the function returns true, then the search is "
          "terminated and thus not all the automorphisms may have been "
          "generated. The *terminate* function may be used to limit the time "
          "spent in bliss in case the graph is too difficult under the "
          "available time constraints. If used, keep the function simple to "
          "evaluate so that it does not consume too much time. ")
      .def(
          "get_permutation_to_canonical_form",
          [](Graph &self, Stats &stats,
             const std::function<void(
                 int, nb::ndarray<nb::ro, uint32_t, nb::ndim<1>, nb::numpy,
                                  nb::c_contig>)> &py_report = nullptr,
             const std::function<bool()> &py_terminate = nullptr) {
            std::function<void(unsigned int, const unsigned int *)> cpp_report =
                nullptr;
            std::function<bool()> cpp_terminate = nullptr;

            if (py_report) {
              cpp_report = [&](unsigned int n, const unsigned int *aut) {
                auto np_aut =
                    nb::ndarray<nb::ro, uint32_t, nb::ndim<1>, nb::numpy,
                                nb::c_contig>(aut, {self.get_nof_vertices()});
                py_report(n, np_aut);
              };
            }

            if (py_terminate) {
              cpp_terminate = [&]() { return py_terminate(); };
            }

            auto perm = self.canonical_form(stats, cpp_report, cpp_terminate);
            // FIXME: Wrap perm in a Numpy class.
          },
          "stats"_a, "report"_a = nb::none(), "terminate"_a = nb::none(),
          "Returns `P`, a :class:`numpy.ndarray` on {0, ..., nvertices-1}. "
          "Applying the 'permutation `P` to this graph results in this graph's "
          "canonical graph. The function *report* (if not None) is called each "
          "time a new  generator for the automorphism group is found. The "
          "first argument  *n* for the function is the length of the "
          "automorphism (equal to  get_nof_vertices()), and the second "
          "argument *aut* is the  automorphism (a bijection on "
          "{0,...,nvertices-1}). *aut* is a read-only :class:`numpy.ndarray`."
          " Do not call any  member functions from the *report* function.\n\n"

          "The search statistics are copied in *stats*.\n\n"

          "If the *terminate* function argument is given, it is called in each "
          "search tree node: if the function returns true, then the search is "
          "terminated and thus not all the automorphisms may have been "
          "generated. The *terminate* function may be used to limit the time "
          "spent in bliss in case the graph is too difficult under the "
          "available time constraints. If used, keep the function simple to "
          "evaluate so that it does not consume too much time.\n\n"

          "This wraps Graph::canonical_form from the C++-API.")
      // FIXME: Make from_dimacs compile-able.
      // .def_static(
      //     "from_dimacs",
      //     [](nb::object file_obj) {
      //       FILE *fp = get_fp_from_writeable_pyobj(file_obj);
      //       Graph *ptr = nullptr;
      //       auto err_str = capture_string_written_to_file(
      //           [&](FILE *err_stream) { ptr = Graph::read_dimacs(fp,
      //           err_stream); });
      //       if (!ptr) {
      //         throw std::runtime_error(
      //             "Error during reading Graph from DIMACS.\n" + err_str);
      //       }
      //       return std::unique_ptr<Graph *>(ptr);
      //     },
      //     "fp"_a,
      //     "Return a graph corresponding to DIMACS-formatted graph present in
      //     "
      //     "*fp*. See the `bliss website "
      //     "<https://users.aalto.fi/tjunttil/bliss>` for the definition of the
      //     " "file format. Note that in the DIMACS file the vertices are
      //     numbered " "from 1 to N while in this API they are from 0 to N-1.
      //     Thus the " "vertex n in the file corresponds to the vertex n-1 in
      //     the API.\n\n"
      //     ":arg fp: The file stream from where the graph is to be read.")
      .def(
          "write_dimacs",
          [](Graph &self, nb::object file_obj) {
            FILE *fp = get_fp_from_writeable_pyobj(file_obj);
            self.write_dimacs(fp);
          },
          "fp"_a,
          "Write the graph to *fp* in a variant of the DIMACS format. See "
          "the `bliss website <https://users.aalto.fi/tjunttil/bliss>` for the "
          "definition of the file format. Note that in the DIMACS file the "
          "vertices are numbered from 1 to N while in this API they are from 0 "
          "to N-1. Thus the vertex n in the file corresponds to the vertex n-1 "
          "in the API.\n\n"
          ":arg fp: The file stream where the graph is to be written.")

      .def(
          "write_dot",
          [](Graph &self, nb::object file_obj) {
            FILE *fp = get_fp_from_writeable_pyobj(file_obj);
            self.write_dot(fp);
          },
          "fp"_a,
          "Write the graph to *fp* in the graphviz format.\n\n"
          ":arg fp: The file stream where the graph is to be written.")
      .def("__hash__", &Graph::get_hash);

  // }}}
}
