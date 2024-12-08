cmake_minimum_required(VERSION 3.19)

# cmake_policy(SET CMP0135 OLD)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -g")

# Compiler flags
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(sysy
  VERSION 0.0.0.1
  DESCRIPTION "The SysY language compiler"
  LANGUAGES CXX
)

# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
# set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g")
set(CMAKE_CXX_EXTENSIONS OFF)

# written by TXS
set(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/bin)

# end
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Build type not set, falling back to Debug mode.")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Choose the type of build, options are: Debug Release." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)

# ANTLR
set(ANTLR_DIR ${THIRD_PARTY_DIR}/antlr4)
set(ANTLR_CPP_RUNTIME_SOURCE_DIR ${ANTLR_DIR}/antlr4-cpp-runtime-4.12.0-source)
set(ANTLR_EXECUTABLE ${ANTLR_DIR}/antlr-4.12.0-complete.jar)

# local install of ANTLR4 runtime
include_directories(/usr/local/include/antlr4-runtime/)
link_directories(/usr/local/lib/)

# Add All Includes
include_directories(
  ${CMAKE_BINARY_DIR}/src/antlr4
  ${CMAKE_SOURCE_DIR}/include/
)

# Project source files
add_subdirectory(src)

# Add Src Files
file(GLOB_RECURSE ALL_SRC_FILES
  "${CMAKE_SOURCE_DIR}/src/*.[ch]"
  "${CMAKE_SOURCE_DIR}/src/*.[ch]pp"
  "${CMAKE_SOURCE_DIR}/include/*.[ch]"
  "${CMAKE_SOURCE_DIR}/include/*.[ch]pp")

find_package(Python3 REQUIRED)

set(SUBMIT_DIR ${CMAKE_CURRENT_BINARY_DIR}/submit)

add_custom_target(submit 
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${SUBMIT_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${SUBMIT_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/submit/CMakeLists.txt ${SUBMIT_DIR}/CMakeLists.txt
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src ${SUBMIT_DIR}/src
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/include ${SUBMIT_DIR}/include
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/antlr4/*.cpp ${SUBMIT_DIR}/src/
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/src/antlr4/*.h ${SUBMIT_DIR}/include/
  COMMAND ${CMAKE_COMMAND} -E rm -rf ${SUBMIT_DIR}/src/runtime

  # file_filter: delete unnecessary files
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/submit/file_filter.py ${SUBMIT_DIR}

  # header_fix: fix include path
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/submit/header_fix.py ${SUBMIT_DIR}

  # compile: compile runtime
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/src/runtime/compile.py RISCV ${SUBMIT_DIR}/include/autogen/riscv/Runtime.hpp

  # copy *.py
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/submit/*.py ${SUBMIT_DIR}/

  # comile: compile compiler
  COMMAND cd ${SUBMIT_DIR}

  # COMMAND ${Python3_EXECUTABLE} ${SUBMIT_DIR}/compile.py ${SUBMIT_DIR} compiler

  # COMMAND make
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/submit/.gitignore ${SUBMIT_DIR}/.gitignore

  # run test
  # COMMAND ${Python3_EXECUTABLE} ${SUBMIT_DIR}/runtest.py ${SUBMIT_DIR}/compiler ${CMAKE_SOURCE_DIR}/test ${CMAKE_SOURCE_DIR}/.tmp/asm ${CMAKE_SOURCE_DIR}/.tmp/exe ${CMAKE_SOURCE_DIR}/.tmp/c

  # compile all performance cases
  # COMMAND ${Python3_EXECUTABLE} ${SUBMIT_DIR}/run.py ${CMAKE_SOURCE_DIR}/compiler ${CMAKE_SOURCE_DIR}/test ${SUBMIT_DIR}/.output
  DEPENDS compiler
)
