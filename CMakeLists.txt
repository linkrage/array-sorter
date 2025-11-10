cmake_minimum_required(VERSION 3.12)
project(array_sorter VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Python
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)

if (Python3_SITEARCH)
    set(ARRAY_SORTER_PYTHON_DESTINATION "${Python3_SITEARCH}")
else()
    set(ARRAY_SORTER_PYTHON_DESTINATION
        "${CMAKE_INSTALL_LIBDIR}/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages")
endif()

execute_process(
    COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
    OUTPUT_VARIABLE pybind11_DIR
    RESULT_VARIABLE pybind11_dir_result
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (pybind11_dir_result)
    message(FATAL_ERROR "pybind11 Python module not found. Please run `pip install -r requirements.txt` first.")
endif()
find_package(pybind11 CONFIG REQUIRED PATHS ${pybind11_DIR})

# C++ library
add_library(cpp_sort STATIC
    src/cpp/sort.cpp
)

target_include_directories(cpp_sort PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
)

# Python module
pybind11_add_module(array_sorter
    src/python/bindings.cpp
)

target_link_libraries(array_sorter PRIVATE
    cpp_sort
    pybind11::module
)
target_include_directories(array_sorter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
)

# Installation
install(TARGETS array_sorter
    LIBRARY DESTINATION ${ARRAY_SORTER_PYTHON_DESTINATION}
    RUNTIME DESTINATION ${ARRAY_SORTER_PYTHON_DESTINATION}
    ARCHIVE DESTINATION ${ARRAY_SORTER_PYTHON_DESTINATION}
)

install(TARGETS cpp_sort
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES src/cpp/sort.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/array_sorter)

# Testing
enable_testing()

include(CTest)

FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(catch2)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

add_executable(test_sort_cpp tests/cpp/test_sort.cpp)
target_link_libraries(test_sort_cpp PRIVATE cpp_sort Catch2::Catch2WithMain)
target_include_directories(test_sort_cpp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp
)
catch_discover_tests(test_sort_cpp)

# Python tests via CTest
add_custom_target(python-tests
    COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_SOURCE_DIR}
            ${CMAKE_COMMAND} -E env "PYTHONPATH=$<TARGET_FILE_DIR:array_sorter>"
            ${Python3_EXECUTABLE} -m pytest tests/python -v
    DEPENDS array_sorter
    COMMENT "Running Python unit tests"
)

add_test(NAME python_sort_tests
    COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=$<TARGET_FILE_DIR:array_sorter>"
            ${Python3_EXECUTABLE} -m pytest ${CMAKE_SOURCE_DIR}/tests/python -v
)
set_tests_properties(python_sort_tests PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
