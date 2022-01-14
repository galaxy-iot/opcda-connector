# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("opcda-client")

# 包含子项目。
add_subdirectory ("opcda-client")

add_executable(main 
	"opcda-client/demo.cpp"
	"opcda-client/opcda_client.h"
	"opcda-client/opcda_client.cpp"
	"opcda-client/opccomn.h"
	"opcda-client/opccomn_i.c"
	"opcda-client/opcda.h"
	"opcda-client/opcda_i.c"
	"opcda-client/opc_enum_i.c"
	"opcda-client/opc_enum.h"
	
	"opcda-client/opcda_item.cpp"
)
