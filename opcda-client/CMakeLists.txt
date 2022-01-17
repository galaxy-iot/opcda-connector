# CMakeList.txt: opcda-client 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 将源代码添加到此项目的可执行文件。
add_executable (opcda-client "opcda_client.cpp" "opcda_client.h" "demo.cpp" "opcda_item.cpp"  "opcda_item.h" "json.h" "json_rpc.h" "json_rpc.cpp" "encoding.h" "encoding.cpp")

# TODO: 如有需要，请添加测试并安装目标。
