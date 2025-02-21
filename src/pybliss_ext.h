#pragma once
#include <functional>
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>

namespace nb = nanobind;

using namespace nb::literals;

// {{{ utils

void perform_sanity_checks_on_perm_array(
    const nb::ndarray<uint32_t, nb::ndim<1>> &ary, size_t reqd_size);
FILE *get_fp_from_writeable_pyobj(nb::object file_obj);
FILE *get_fp_from_readable_pyobj(nb::object file_obj);
std::string
capture_string_written_to_file(std::function<void(FILE *)> file_writer);

// }}}

// Bind bliss classes
void bind_bignum(nb::module_ &m);
void bind_stats(nb::module_ &m);
void bind_graph(nb::module_ &m);
void bind_digraph(nb::module_ &m);
void bind_utils(nb::module_ &m);
