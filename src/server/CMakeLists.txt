# 指定SRC_LIST,包含该目录下所有源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
add_executable(ChatServer ${SRC_LIST} ${DB_LIST})

target_link_libraries(ChatServer muduo_net muduo_base mysqlclient pthread)

