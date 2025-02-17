#include <bliss/graph.hh>
#include <bliss/stats.hh>
#include <cstring>
#include <nanobind/nanobind.h>
#include <pybliss_ext.h>

using namespace bliss;

void bind_graph(nb::module_ &m) {
  nb::class_<Graph>(m, "Graph",
                    "Undirected vertex-colored graph.\n\n"
                    "See :class:`DiGraph` for a directed variant.")
      .def(nb::init<>())
      .def(nb::init<unsigned int>())
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
              cpp_terminate = py_terminate;
            }

            auto perm = self.canonical_form(stats, cpp_report, cpp_terminate);
            nb::module_ np = nb::module_::import_("numpy");
            auto np_perm =
                np.attr("empty")(self.get_nof_vertices(), np.attr("uint32"));
            std::memcpy(nb::cast<nb::ndarray<>>(np_perm).data(), perm,
                        sizeof(uint32_t) * self.get_nof_vertices());
            return np_perm;
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
      .def_static(
          "from_dimacs",
          [](nb::object file_obj) {
            FILE *fp = get_fp_from_writeable_pyobj(file_obj);
            Graph *ptr = nullptr;
            auto err_str =
                capture_string_written_to_file([&](FILE *err_stream) {
                  ptr = Graph::read_dimacs(fp, err_stream);
                });
            if (!ptr) {
              throw std::runtime_error(
                  "Error during reading Graph from DIMACS.\n" + err_str);
            }
            return ptr;
          },
          "fp"_a,
          "Return a graph corresponding to DIMACS-formatted graph present in "
          "*fp*. See the `bliss website "
          "<https://users.aalto.fi/tjunttil/bliss>` for the definition of the "
          "file format. Note that in the DIMACS file the vertices are numbered "
          "from 1 to N while in this API they are from 0 to N-1. Thus the "
          "vertex n in the file corresponds to the vertex n-1 in the API.\n\n"
          ":arg fp: The file stream from where the graph is to be read.")
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
      .def("copy", &Graph::copy, "Returns a copy of this graph.")
      .def("cmp", &Graph::cmp, "other"_a,
           "Compare this graph to *other* in a total order on graphs. Returns "
           "0 if graphs are equal, -1 if this graph is \"smaller than\" the "
           "other, and 1 if this graph is \"greater than\" *other*.")
      .def(
          "show_dot",
          [](Graph &self, nb::object output_to) {
            nb::module_ pytools_graphviz =
                nb::module_::import_("pytools.graphviz");
            const std::string dot_code = capture_string_written_to_file(
                [&](FILE *fp) { self.write_dot(fp); });
            pytools_graphviz.attr("show_dot")(nb::str(dot_code.c_str()),
                                              output_to);
          },
          "output_to"_a = nb::none(),
          "Visualize the graph.\n\n"
          ":arg output_to:  Passed on to :func:`pytools.graphviz.show_dot` "
          "unmodified.")
      .def("__hash__", &Graph::get_hash);
}