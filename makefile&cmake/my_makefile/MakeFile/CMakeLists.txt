cmake_minimum_required (VERSION 2.8) #最低版本要求

project(test1)   #项目名称

include_directories(./include)   #添加头文件路径

aux_source_directory(lib LIB_LIST) #获取./lib中的源文件

add_executable (hello hello.c ${LIB_LIST})   #生成可执行文件

