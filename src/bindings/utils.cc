#include "pybliss_ext.h"
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
