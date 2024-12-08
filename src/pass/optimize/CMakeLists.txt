project(pass_optimize)

# only include the source files in the directory
# aux_source_directory(. PASS_SRC_FILES)

# recursive globbing to include all files in the directory
file(GLOB_RECURSE PASS_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
add_library(optimize SHARED ${PASS_SRC_FILES})