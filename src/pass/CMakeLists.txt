project(pass)

add_subdirectory("analysis")
add_subdirectory("optimize")

aux_source_directory(. PASS_SRC_FILES)

add_library(pass
    SHARED ${PASS_SRC_FILES}
)


# target_link_libraries(pass PUBLIC ir)
target_link_libraries(pass PUBLIC analysis)
target_link_libraries(pass PUBLIC optimize)
