cmake_minimum_required(VERSION 3.0)
project(main)

# 配置编译选项 -o/-g
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

# 配置头文件搜索路径
# include_directories()

# 配置库文件搜索路径
# link_directories()

# 设置需要编译的源文件列表
set(SRC_LIST muduo_server.cpp)

# 设置可执行文件存储路径
set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 把指定路径下的所有源文件放入变量名
# aux_source_directory(. STC_LIST)

# 生成可执行文件server,由SRC_LIST定义源文件
add_executable(server ${SRC_LIST})

# 链接库 server目标程序需要的链接库，muduo_net,muduo_base,pthread
target_link_libraries(server muduo_net muduo_base pthread)
