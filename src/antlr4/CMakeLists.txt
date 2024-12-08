# Generate lexer and parser with ANTLR
list(APPEND CMAKE_MODULE_PATH ${ANTLR_CPP_RUNTIME_SOURCE_DIR}/cmake)

message(STATUS "CMake module path for ANTLR runtime found in ${CMAKE_MODULE_PATH}")

find_package(ANTLR REQUIRED)

antlr_target(SysYGen SysY.g4
  LEXER PARSER
  OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  VISITOR
)

add_library(SysYParser SHARED ${ANTLR_SysYGen_CXX_OUTPUTS})
target_link_libraries(SysYParser PUBLIC antlr4-runtime)

