# aux_source_directory(.SRC_LIST)
add_executable(ChatClient main.cpp)

target_link_libraries(ChatClient pthread)