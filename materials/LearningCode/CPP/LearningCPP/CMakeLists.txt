cmake_minimum_required(VERSION 3.20)
project(LearningCPP)

# 1. 自动管理 Eigen 5.0.0 依赖 (无需手动下载，无需担心路径)
include(FetchContent)
FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 5.0.0
)
FetchContent_MakeAvailable(eigen)

# 2. 项目全局设置
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

# 3. 定义辅助函数：减少重复代码，自动关联 Eigen
function(add_learning_task name source)
    add_executable(${name} ${source})
    # 将 Eigen 自动链接到每个目标上，无需手动 include_directories
    target_link_libraries(${name} PRIVATE Eigen3::Eigen)
endfunction()

# 4. 为每个练习文件创建独立的运行目标
add_learning_task(MainTask main.cpp)
add_learning_task(ListSort listsort.cpp)
add_learning_task(TestFile testfile.cpp)
add_learning_task(WriteFile writefile.cpp)