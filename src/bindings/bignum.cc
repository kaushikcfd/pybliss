#include <bliss/bignum.hh>
#include <nanobind/nanobind.h>
#include <pybliss_ext.h>

using namespace bliss;

void bind_bignum(nb::module_ &m) {
  nb::class_<BigNum>(m, "BigNum",
                     "A wrapper class for non-negative big integers.")
      .def(nb::init<>())
      .def("assign", &BigNum::assign)
      .def("multiply", &BigNum::multiply)
      .def("print_to_file",
           [](BigNum &self, nb::object fp_obj) {
             FILE *fp = get_fp_from_writeable_pyobj(fp_obj);
             self.print(fp);
           })
      // FIXME: Add a to_python method.
      .def("__str__", [](BigNum &self) {
        return capture_string_written_to_file(
            [&](FILE *fp) { self.print(fp); });
      });
}