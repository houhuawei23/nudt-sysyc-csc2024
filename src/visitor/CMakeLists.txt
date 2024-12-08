project(visitor)
aux_source_directory(. VISITOR_SRC_FILES)

add_library(visitor
    SHARED ${VISITOR_SRC_FILES}
)

target_link_libraries(visitor PUBLIC 
    SysYParser 
    ir
) # PRIVATE -> PUBLIC 
