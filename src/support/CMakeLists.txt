
project(support)

aux_source_directory(. SUPPORT_SRC_FILES)

add_library(support
    SHARED ${SUPPORT_SRC_FILES}
)

