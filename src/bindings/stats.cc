#include <bliss/bignum.hh>
#include <bliss/graph.hh>
#include <bliss/stats.hh>
#include <nanobind/nanobind.h>
#include <pybliss_ext.h>

using namespace bliss;

void bind_stats(nb::module_ &m) {
  nb::class_<Stats>(m, "Stats",
                    "Records statistics returned by the search algorithms.\n\n"
                    ".. automethod:: __init__\n"
                    ".. automethod:: print_to_file\n"
                    ".. autoattribute:: group_size\n"
                    ".. autoattribute:: group_size_as_bignum\n"
                    ".. autoattribute:: group_size_approx\n"
                    ".. autoattribute:: n_nodes\n"
                    ".. autoattribute:: n_leaf_nodes\n"
                    ".. autoattribute:: n_bad_nodes\n"
                    ".. autoattribute:: n_canupdates\n"
                    ".. autoattribute:: n_generators\n"
                    ".. autoattribute:: max_level\n"
                    ".. automethod:: __str__")
      .def(nb::init<>())
      .def("print_to_file",
           [](Stats &self, nb::object fp_obj) {
             FILE *fp = get_fp_from_writeable_pyobj(fp_obj);
             self.print(fp);
             fflush(fp);
           })
      .def_prop_ro(
          "group_size",
          [](Stats &self) {
            auto grp_size_str = capture_string_written_to_file(
                [&](FILE *fp) { self.get_group_size().print(fp); });
            return nb::int_(nb::str(grp_size_str.c_str()));
          },
          "The size of the automorphism group as :class:`int`.")
      .def_prop_ro(
          "group_size_as_bignum",
          [](Stats &self) { return self.get_group_size(); },
          "The size of the automorphism group as :class:`BigNum`.")
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
        return nb::str(capture_string_written_to_file([&](FILE *fp) {
                         self.print(fp);
                       }).c_str());
      });
}
