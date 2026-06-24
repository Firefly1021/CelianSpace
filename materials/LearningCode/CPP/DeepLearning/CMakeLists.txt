cmake_minimum_required(VERSION 3.20)
project(DeepLearning)

# 核心：向上跳三级找到 libtorch
set(LIBTORCH_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../libtorch")

# 验证路径是否存在（跨平台调试利器，省去猜路径的烦恼）
if(NOT EXISTS "${LIBTORCH_PATH}")
    message(FATAL_ERROR "找不到 LibTorch!请检查路径: ${LIBTORCH_PATH}")
endif()

# 告诉 CMake 在这里寻找 TorchConfig.cmake
set(CMAKE_PREFIX_PATH "${LIBTORCH_PATH}")

find_package(Torch REQUIRED)

# 标准配置
set(CMAKE_CXX_STANDARD 17)
add_executable(DeepLearning main.cpp)
target_link_libraries(DeepLearning "${TORCH_LIBRARIES}")

# Windows 自动拷贝 DLL 逻辑 (跨设备必带)
if(MSVC)
    add_custom_command(TARGET DeepLearning POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${TORCH_INSTALL_PREFIX}/lib/*.dll"
        "$<TARGET_FILE_DIR:DeepLearning>")
endif()