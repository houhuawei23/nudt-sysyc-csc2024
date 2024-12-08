project(pass_analysis)

aux_source_directory(. ANALYSIS_SRC_FILES)
file(GLOB_RECURSE ANALYSIS_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

add_library(analysis SHARED ${ANALYSIS_SRC_FILES})

# target_include_directories(analysis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)

