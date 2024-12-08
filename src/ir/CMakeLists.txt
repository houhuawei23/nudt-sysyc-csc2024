
project(ir)

aux_source_directory(. IR_SRC_FILES)

add_library(ir
    SHARED ${IR_SRC_FILES}
)

