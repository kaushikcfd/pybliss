#include <bliss/graph.hh>
#include <bliss/stats.hh>
#include <cstring>
#include <nanobind/nanobind.h>
#include <nanobind/stl/function.h>
#include <nanobind/stl/optional.h>
#include <optional>
#include <pybliss_ext.h>

using namespace bliss;

void bind_graph(nb::module_ &m) {
  nb::class_<Graph> graph(m, "Graph", R"(
                          Undirected vertex-colored graph. See ``DiGraph`` for a directed variant.

                          .. automethod:: __init__
                          .. automethod:: set_verbose_level
                          .. automethod:: set_verbose_file
                          .. automethod:: add_vertex
                          .. automethod:: add_edge
                          .. automethod:: get_color
                          .. automethod:: change_color
                          .. automethod:: set_failure_recording
                          .. automethod:: set_component_recursion
                          .. autoattribute:: nvertices
                          .. automethod:: permute
                          .. automethod:: is_automorphism
                          .. automethod:: find_automorphisms
                          .. automethod:: get_permutation_to_canonical_form
                          .. automethod:: write_dimacs
                          .. automethod:: write_dot
                          .. automethod:: show_dot
                          .. automethod:: from_dimacs
                          .. automethod:: copy
                          .. automethod:: cmp
                          .. automethod:: set_long_prune_activity
                          .. automethod:: set_splitting_heuristic)");
  graph.def(nb::init<>());
  graph.def(nb::init<unsigned int>());
  graph.def(
      "set_verbose_level", &Graph::set_verbose_level, "level"_a,
      "Set the verbose output level for the algorithms\n"
      ":arg level: The level of verbose output, 0 means no verbose output.");
  graph.def(
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
      nb::keep_alive<1, 2>(), "fp"_a,
      "Set the file stream for verbose output.\n\n"
      ":param file_obj: The file object to write the output to. If None, "
      "writing to the file is disabled.");
  graph.def("add_vertex", &Graph::add_vertex, "color"_a = 0,
            "Add a new vertex with color *color* and return its new index.");
  graph.def("add_edge", &Graph::add_edge, "v1"_a, "v2"_a,
            "Add an edge between *v1* and *v2*.");
  graph.def("get_color", &Graph::get_color, "v"_a,
            "Returns the color of the vertex *v*");
  graph.def("change_color", &Graph::change_color, "v"_a, "c"_a,
            "Change the color of vertex *v* to *c*.");
  graph.def("set_failure_recording", &Graph::set_failure_recording, "active"_a,
            "Activate / deactivate failure recording\n\n"
            ":arg active:If true, activate failure recording, deactivate "
            "otherwise.");
  graph.def(
      "set_component_recursion", &Graph::set_component_recursion, "active"_a,
      "Activate/deactivate component recursion. The choice affects the "
      "computed canonical labelings; therefore, if you want to compare "
      "whether two graphs are isomorphic by computing and comparing (for "
      "equality) their canonical versions, be sure to use the same choice "
      "for both graphs. May not be called during the search, i.e. from an "
      "automorphism reporting hook function.\n\n"
      ":arg active:  If true, activate component recursion, deactivate "
      "otherwise.");
  graph.def_prop_ro(
      "nvertices", [](Graph &self) { return self.get_nof_vertices(); },
      "Return the number of vertices in the graph.");
  graph.def(
      "permute",
      [](Graph &self, const nb::ndarray<uint32_t, nb::ndim<1>> &ary) {
        perform_sanity_checks_on_perm_array(ary, self.get_nof_vertices());
        return self.permute((uint32_t *)ary.data());
      },
      "perm"_a,
      " Return a new graph that is the result of applying the permutation "
      "*perm* to this graph. This graph is not modified. *perm* must "
      "contain N=this.get_nof_vertices() elements and be a bijection on "
      "{0,1,...,N-1}, otherwise the result is undefined.");
  graph.def(
      "is_automorphism",
      [](Graph &self, const nb::ndarray<uint32_t, nb::ndim<1>> &ary) {
        perform_sanity_checks_on_perm_array(ary, self.get_nof_vertices());
        return self.is_automorphism((uint32_t *)ary.data());
      },
      "perm"_a,
      " Return true only if *perm* is an automorphism of this graph."
      " *perm* must contain N=this.get_nof_vertices() elements and be a"
      " bijection on {0,1,...,N-1}, otherwise the result is undefined.");
  graph.def(
      "find_automorphisms",
      [](Graph &self, Stats &stats,
         std::optional<const std::function<void(
             int, nb::ndarray<nb::ro, uint32_t, nb::ndim<1>, nb::numpy,
                              nb::c_contig>)>> &py_report,
         std::optional<const std::function<bool()>> &py_terminate) {
        std::function<void(unsigned int, const unsigned int *)> cpp_report =
            nullptr;
        std::function<bool()> cpp_terminate = nullptr;

        if (py_report) {
          cpp_report = [&](unsigned int n, const unsigned int *aut) {
            auto np_aut =
                nb::ndarray<nb::ro, uint32_t, nb::ndim<1>, nb::numpy,
                            nb::c_contig>(aut, {self.get_nof_vertices()});
            (*py_report)(n, np_aut);
          };
        }

        if (py_terminate) {
          cpp_terminate = *py_terminate;
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
      "read-only :class:`numpy.ndarray`. Additionally *aut*'s entries are"
      " invalidated across calls to *terminate*. Caller must copy *aut* if"
      " long-term usage is intended. Do not call any member functions "
      "from the *report* function.\n\n"

      "The search statistics are copied in *stats*.\n\n"

      "If the *terminate* function argument is given, it is called in each "
      "search tree node: if the function returns true, then the search is "
      "terminated and thus not all the automorphisms may have been "
      "generated. The *terminate* function may be used to limit the time "
      "spent in bliss in case the graph is too difficult under the "
      "available time constraints. If used, keep the function simple to "
      "evaluate so that it does not consume too much time. ");
  graph.def(
      "get_permutation_to_canonical_form",
      [](Graph &self, Stats &stats,
         std::optional<const std::function<void(
             int, nb::ndarray<nb::ro, uint32_t, nb::ndim<1>, nb::numpy,
                              nb::c_contig>)>> &py_report,
         std::optional<const std::function<bool()>> &py_terminate) {
        std::function<void(unsigned int, const unsigned int *)> cpp_report =
            nullptr;
        std::function<bool()> cpp_terminate = nullptr;

        if (py_report) {
          cpp_report = [&](unsigned int n, const unsigned int *aut) {
            auto np_aut =
                nb::ndarray<nb::ro, uint32_t, nb::ndim<1>, nb::numpy,
                            nb::c_contig>(aut, {self.get_nof_vertices()});
            (*py_report)(n, np_aut);
          };
        }

        if (py_terminate) {
          cpp_terminate = *py_terminate;
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
      "{0,...,nvertices-1}). *aut* is a read-only :class:`numpy.ndarray`. "
      "Additionally *aut*'s entries are invalidated across calls to "
      "*terminate*. Caller must copy *aut* if long-term usage is intended."
      " Do not call any  member functions from the *report* function.\n\n"

      "The search statistics are copied in *stats*.\n\n"

      "If the *terminate* function argument is given, it is called in each "
      "search tree node: if the function returns true, then the search is "
      "terminated and thus not all the automorphisms may have been "
      "generated. The *terminate* function may be used to limit the time "
      "spent in bliss in case the graph is too difficult under the "
      "available time constraints. If used, keep the function simple to "
      "evaluate so that it does not consume too much time.\n\n"

      "This wraps Graph::canonical_form from the C++-API.");
  graph.def_static(
      "from_dimacs",
      [](nb::object file_obj) {
        FILE *fp = get_fp_from_readable_pyobj(file_obj);
        Graph *ptr = nullptr;
        auto err_str = capture_string_written_to_file([&](FILE *err_stream) {
          ptr = Graph::read_dimacs(fp, err_stream);
        });
        if (!ptr) {
          throw std::runtime_error("Error during reading Graph from DIMACS.\n" +
                                   err_str);
        }
        return ptr;
      },
      "fp"_a,
      "Return a graph corresponding to DIMACS-formatted graph present in *fp*. "
      "See the `bliss website <https://users.aalto.fi/tjunttil/bliss>` for the "
      "definition of the file format. Note that in the DIMACS file the "
      "vertices are numbered from 1 to N while in this API they are from 0 to "
      "N-1. Thus the vertex n in the file corresponds to the vertex n-1 in the "
      "API.\n\n"
      ":arg fp: The file stream from where the graph is to be read.");
  graph.def(
      "write_dimacs",
      [](Graph &self, nb::object file_obj) {
        FILE *fp = get_fp_from_writeable_pyobj(file_obj);
        self.write_dimacs(fp);
        fflush(fp);
      },
      "fp"_a,
      "Write the graph to *fp* in a variant of the DIMACS format. See the "
      "`bliss website <https://users.aalto.fi/tjunttil/bliss>` for the "
      "definition of the file format. Note that in the DIMACS file the "
      "vertices are numbered from 1 to N while in this API they are from 0 to "
      "N-1. Thus the vertex n in the file corresponds to the vertex n-1 in the "
      "API.\n\n"
      ":arg fp: The file stream where the graph is to be written.");
  graph.def(
      "write_dot",
      [](Graph &self, nb::object file_obj) {
        FILE *fp = get_fp_from_writeable_pyobj(file_obj);
        self.write_dot(fp);
        fflush(fp);
      },
      "fp"_a,
      "Write the graph to *fp* in the graphviz format.\n\n"
      ":arg fp: The file stream where the graph is to be written.");
  graph.def("copy", &Graph::copy, "Returns a copy of this graph.");
  graph.def("cmp", &Graph::cmp, "other"_a,
            "Compare this graph to *other* in a total order on graphs. Returns "
            "0 if graphs are equal, -1 if this graph is \"smaller than\" the "
            "other, and 1 if this graph is \"greater than\" *other*.");
  graph.def(
      "show_dot",
      [](Graph &self, nb::object output_to) {
        nb::module_ pytools_graphviz = nb::module_::import_("pytools.graphviz");
        const std::string dot_code = capture_string_written_to_file(
            [&](FILE *fp) { self.write_dot(fp); });
        pytools_graphviz.attr("show_dot")(nb::str(dot_code.c_str()), output_to);
      },
      "output_to"_a = nb::none(),
      "Visualize the graph.\n\n"
      ":arg output_to:  Passed on to :func:`pytools.graphviz.show_dot` "
      "unmodified.");
  graph.def("__hash__", &Graph::get_hash);
  graph.def("set_long_prune_activity", &Graph::set_long_prune_activity,
            "active"_a,
            "Disable/enable the long prune method. The choice affects the "
            "computed canonical labelings. Therefore, if you want to compare "
            "whether two graphs are isomorphic by computing and comparing (for "
            "equality) their canonical versions, be sure to use the same "
            "choice for both graphs. May not be called during the search, i.e. "
            "from an automorphism reporting hook function. *active*  if true, "
            "activate long prune, deactivate otherwise");
  graph.def("set_splitting_heuristic", &Graph::set_splitting_heuristic, "shs"_a,
            "Set the splitting heuristic used by the automorphism and "
            "canonical labeling algorithm. The selected splitting heuristics "
            "affects the computed canonical labelings. Therefore, if you want "
            "to compare whether two graphs are isomorphic by computing and "
            "comparing (for equality) their canonical versions, be sure to use "
            "the same splitting heuristics for both graphs.");

  nb::enum_<Graph::SplittingHeuristic>(graph, "SplittingHeuristic", R"(
      Enum defining the splitting heuristics for graph canonicalization.

      .. attribute:: shs_f

        First non-unit cell. Very fast but may result in large search
        spaces on difficult graphs. Use for large but easy graphs.

      .. attribute:: shs_fs

        First smallest non-unit cell. Fast, should usually produce smaller
        search spaces than shs_f.

      .. attribute:: shs_fl

        First largest non-unit cell. Fast, should usually produce smaller
        search spaces than shs_f.

      .. attribute:: shs_fm

        First maximally non-trivially connected non-unit cell. Not so
        fast, should usually produce smaller search spaces than shs_f,
        shs_fs, and shs_fl.

      .. attribute:: shs_fsm

        First smallest maximally non-trivially connected non-unit cell.
        Not so fast, should usually produce smaller search spaces than
        shs_f, shs_fs, and shs_fl.

      .. attribute:: shs_flm

        First largest maximally non-trivially connected non-unit cell. Not
        so fast, should usually produce smaller search spaces than shs_f,
        shs_fs, and shs_fl.)")
      .value("shs_f", Graph::SplittingHeuristic::shs_f)
      .value("shs_fs", Graph::SplittingHeuristic::shs_fs)
      .value("shs_fl", Graph::SplittingHeuristic::shs_fl)
      .value("shs_fm", Graph::SplittingHeuristic::shs_fm)
      .value("shs_fsm", Graph::SplittingHeuristic::shs_fsm)
      .value("shs_flm", Graph::SplittingHeuristic::shs_flm)
      .export_values();
}
