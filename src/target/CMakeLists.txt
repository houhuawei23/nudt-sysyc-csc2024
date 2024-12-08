project(target)
aux_source_directory(. TARGET_SRC_FILES)

add_library(target
    SHARED ${TARGET_SRC_FILES}
)

# add_subdirectory(
#     generic
# )
add_subdirectory(
    riscv
)
target_link_libraries(target PUBLIC 
    # generic
    riscv
) 
