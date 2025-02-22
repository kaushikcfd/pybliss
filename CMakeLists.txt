cmake_minimum_required(VERSION 3.15...3.26)
set (CMAKE_CXX_STANDARD 17)

project(pybliss LANGUAGES CXX)

if (NOT SKBUILD)
  message( FATAL_ERROR "This CMake file is meant to be executed using 'scikit-build'" )
endif()

# Try to import all Python components potentially needed by nanobind
find_package(Python 3.8
  REQUIRED COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)

# Import nanobind through CMake's find_package mechanism
find_package(nanobind CONFIG REQUIRED)

set(
  BLISS_SOURCE_FILES
  src/Bliss/src/abstractgraph.cc
  src/Bliss/src/bliss_C.cc
  src/Bliss/src/defs.cc
  src/Bliss/src/digraph.cc
  src/Bliss/src/graph.cc
  src/Bliss/src/orbit.cc
  src/Bliss/src/partition.cc
  src/Bliss/src/uintseqhash.cc
  src/Bliss/src/utils.cc
)

# We are now ready to compile the actual extension module
nanobind_add_module(
  pybliss_ext

  STABLE_ABI

  NB_STATIC

  # Source code:
  src/pybliss_ext.cc
  src/bindings/stats.cc
  src/bindings/bignum.cc
  src/bindings/graph.cc
  src/bindings/utils.cc
  ${BLISS_SOURCE_FILES}
)

target_include_directories(pybliss_ext PRIVATE src/Bliss/include)
target_include_directories(pybliss_ext PRIVATE src/)

# Install directive for scikit-build-core
install(TARGETS pybliss_ext LIBRARY DESTINATION pybliss)

# Generate stubs for user QoL.
nanobind_add_stub(
  pybliss_ext_stub
  MODULE pybliss_ext
  OUTPUT __init__.pyi
  PYTHON_PATH "${CMAKE_INSTALL_PREFIX}/pybliss"
  DEPENDS pybliss_ext
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/__init__.pyi DESTINATION pybliss)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/pybliss/py.typed DESTINATION pybliss)
