#include <nanobind/nanobind.h>
#include <pybliss_ext.h>

NB_MODULE(pybliss_ext, m) {
  m.doc() = "Wrapper for BLISS-toolkit for graph canonicalization.";

  bind_bignum(m);
  bind_stats(m);
  bind_graph(m);
  bind_utils(m);
}
