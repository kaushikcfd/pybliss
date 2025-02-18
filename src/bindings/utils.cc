#include "pybliss_ext.h"
#include <bliss/utils.hh>
#include <memory>
#include <nanobind/nanobind.h>

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

/**
 * Grabs the FILE* from \p py_file for writing into the file-descriptor
 * pointed by \p py_file. We do not have any sane deleters associated with
 * the returned file pointer. Caller must manage lifetime appropriately.
 */
FILE *get_fp_from_writeable_pyobj(nb::object py_file) {
  if (!nb::hasattr(py_file, "fileno")) {
    throw std::runtime_error(
        "Expected a file-like object with a 'fileno' method");
  }

  if (!nb::hasattr(py_file, "write")) {
    throw std::runtime_error(
        "File object is not writable (missing 'write' method)");
  }
  py_file.attr("flush")(); // Flush any existing buffers.

  int fd = nb::cast<int>(py_file.attr("fileno")());
  FILE *file = fdopen(fd, "w");
  if (!file) {
    throw std::runtime_error("Failed to convert file descriptor to FILE*");
  }
  return file;
}

/**
 * Grabs the FILE* from \p py_file for reading from the file-descriptor
 * pointed by \p py_file. The returned file-pointer is seeked up to the
 * point where the python IOReader has read. We do not have any sane
 * deleters associated with the returned file pointer. Caller must manage
 * lifetime appropriately.
 */
FILE *get_fp_from_readable_pyobj(nb::object py_file) {
  // Check if the object has a 'fileno' method
  if (!nb::hasattr(py_file, "fileno")) {
    throw std::runtime_error(
        "Expected a file-like object with a 'fileno' method");
  }

  // Check if the object is writable
  if (!nb::hasattr(py_file, "read")) {
    throw std::runtime_error(
        "File object is not readable (missing 'read' method)");
  }
  int fd = nb::cast<int>(py_file.attr("fileno")());
  FILE *file = fdopen(fd, "r");
  if (!file) {
    throw std::runtime_error("Failed to convert file descriptor to FILE*");
  }
  fseek(file, nb::cast<long int>(py_file.attr("tell")()), SEEK_SET);
  return file;
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

void bind_utils(nb::module_ &m) {
  m.def(
      "print_permutation_to_file",
      [](nb::object fp_obj, const nb::ndarray<uint32_t, nb::ndim<1>> &ary,
         uint32_t offset) {
        FILE *fp = get_fp_from_writeable_pyobj(fp_obj);
        bliss::print_permutation(fp, ary.shape(0), (uint32_t *)ary.data(),
                                 offset);
        fflush(fp);
      },
      "fp"_a, "perm"_a, "offset"_a = 0,
      "Print the permutation in the cycle format in the file stream *fp*. The "
      "amount *offset* is added to each element before printing, e.g. the "
      "permutation (2 4) is printed as (3 5) when *offset* is 1. Wraps "
      "``print_permutation`` from the C++-API. Also see "
      ":func:`permutation_to_str`.");

  m.def(
      "permutation_to_str",
      [](const nb::ndarray<uint32_t, nb::ndim<1>> &ary, uint32_t offset) {
        auto perm_str = capture_string_written_to_file([&](FILE *fp) {
          bliss::print_permutation(fp, ary.shape(0), (uint32_t *)ary.data(),
                                   offset);
        });
        return nb::str(perm_str.c_str());
      },
      "perm"_a, "offset"_a = 0,
      "Returns a :class:`str` corresponding to the permutation *perm* in cycle "
      "format. The amount *offset* is added to each element before "
      "stringifying, e.g. the permutation (2 4) is printed as (3 5) when "
      "*offset* is 1. Also see :func:`print_permutation_to_file`.");
}