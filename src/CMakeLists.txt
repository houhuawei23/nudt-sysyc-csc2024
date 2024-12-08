project(compiler)

# add all src files in current directory
aux_source_directory(. SRC_FILES)

# add subdirectory
add_subdirectory(antlr4)
add_subdirectory(ir)
add_subdirectory(visitor)
add_subdirectory(pass)
add_subdirectory(mir)
add_subdirectory(support)
add_subdirectory(target)

add_executable(compiler
    ${SRC_FILES}
)

target_link_libraries(compiler PUBLIC 
    ir
    visitor  
    pass
    mir
    support
    target
    m
    antlr4-runtime
)
