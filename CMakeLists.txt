cmake_minimum_required(VERSION 3.9)
option(FCPP_INTERNAL_TESTS "Build internal tests for FCPP." OFF)
add_subdirectory(./fcpp/src)
fcpp_setup()

project(
    hierarchical_collection
    VERSION 0.0.0
    DESCRIPTION "Experimental evaluation of the hierarchical distributed collection algorithm."
)

#fcpp_target(batch   ./run/batch.cpp   OFF)
fcpp_target(graphic ./run/graphic.cpp ON)
