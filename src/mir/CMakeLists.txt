
project(mir)

# aux_source_directory(. IR_SRC_FILES)
file(GLOB_RECURSE MIR_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

add_library(mir
    SHARED ${MIR_SRC_FILES}
)

