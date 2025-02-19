#include <bliss/bignum.hh>
#include <nanobind/nanobind.h>
#include <pybliss_ext.h>

using namespace bliss;

void bind_bignum(nb::module_ &m) {
  nb::class_<BigNum>(m, "BigNum",
                     "A wrapper class for non-negative big integers.\n\n"
                     ".. automethod:: __init__\n"
                     ".. automethod:: assign\n"
                     ".. automethod:: multiply\n"
                     ".. automethod:: print_to_file\n"
                     ".. automethod:: __str__")
      .def(nb::init<>())
      .def("assign", &BigNum::assign)
      .def("multiply", &BigNum::multiply)
      .def("print_to_file",
           [](BigNum &self, nb::object fp_obj) {
             FILE *fp = get_fp_from_writeable_pyobj(fp_obj);
             self.print(fp);
             fflush(fp);
           })
      // FIXME: Add a to_python method.
      .def("__str__", [](BigNum &self) {
        return nb::str(capture_string_written_to_file([&](FILE *fp) {
                         self.print(fp);
                       }).c_str());
      });
}