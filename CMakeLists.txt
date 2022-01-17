# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

project ("opcda-client")

SET(OPCDA_CLIENT_HEADER 
	"opcda-client/opcda_client.h"
	"opcda-client/opccomn.h"
	"opcda-client/opcda.h"
	"opcda-client/opc_enum.h"
	"opcda-client/opcda_async.h"
	"opcda-client/opcda_item.h"
	"opcda-client/encoding.h"
)

SET(OPCDA_CLIENT_SRC 
	"opcda-client/opcda_client.cpp"
	"opcda-client/opccomn_i.c"
	"opcda-client/opcda_i.c"
	"opcda-client/opc_enum_i.c"
	"opcda-client/opcda_async.cpp"
	"opcda-client/opcda_item.cpp"
	"opcda-client/encoding.cpp"
)

add_library (opcda-client STATIC
	${OPCDA_CLIENT_SRC}
	${OPCDA_CLIENT_HEADER}
)

add_executable(opcda-driver 
	"opcda-client/json.h"
	"opcda-client/json_rpc.cpp"
	"opcda-client/json_rpc.h"
	"opcda-client/json_rpc_msg.cpp"
	"opcda-client/json_rpc_msg.h"
	"opcda-client/opcda_driver.cpp"
)

target_link_libraries(opcda-driver ws2_32)
target_link_libraries(opcda-driver opcda-client)

add_executable(demo 
	"opcda-client/demo.cpp"
)

target_link_libraries(demo opcda-client)