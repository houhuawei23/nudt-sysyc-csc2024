project(riscv)

aux_source_directory(. RISCV_SRC_FILES)

add_library(riscv
    SHARED ${RISCV_SRC_FILES}
)
