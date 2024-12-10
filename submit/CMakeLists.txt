cmake_minimum_required(VERSION 3.10)
project(YOUR_PROJECT_NAME)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

include_directories(/usr/local/include/antlr4-runtime/)
link_directories(/usr/local/lib/)

# include_directories(/usr/local/include/antlr4-runtime/)
# link_directories(/usr/local/lib/)

# Source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
)

# Executable target
add_executable(compiler ${SOURCES})

# Link against m library
target_link_libraries(compiler m)

# Link against antlr4-runtime library
target_link_libraries(compiler antlr4-runtime)