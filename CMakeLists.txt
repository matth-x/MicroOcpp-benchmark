# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(mo_benchmark)

# idf_build_set_property(COMPILE_OPTIONS -DMOCPP_TRAFFIC_OUT APPEND)
idf_build_set_property(COMPILE_OPTIONS -DMOCPP_DBG_LEVEL=MOCPP_DL_NONE APPEND)
idf_build_set_property(COMPILE_OPTIONS -DMOCPP_NUMCONNECTORS=3 APPEND)
idf_build_set_property(COMPILE_OPTIONS -DMOCPP_FILENAME_PREFIX="/mo_store/" APPEND)
