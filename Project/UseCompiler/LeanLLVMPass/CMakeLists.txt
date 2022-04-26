cmake_minimum_required(VERSION 3.13.4)
project(LEANLLVMPASS)

# 检查 LLVM 安装目录
set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM install directory")

# 检查 include 目录
set(LT_LLVM_INCLUDE_DIR "${LT_LLVM_INSTALL_DIR}/include/llvm")
if(NOT EXISTS "${LT_LLVM_INCLUDE_DIR}")
    message(FATAL_ERROR "LLVM include directory not found: ${LT_LLVM_INCLUDE_DIR}")
endif()

# 检查 LLVMConfig.cmake 文件
set(LT_VALID_INSTALLATION FALSE)

# Darwin 或 Ubuntu 系统
if(EXISTS "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/LLVMConfig.cmake")
    set(LT_VALID_INSTALLATION TRUE)
endif()

if(NOT ${LT_VALID_INSTALLATION})
    message(FATAL_ERROR "LLVMConfig.cmake not found in ${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm")
endif()

# 加载 llvm 配置
# http://llvm.org/docs/CMake.html#embedding-llvm-in-your-project
# 将 LLVMConfig.cmake 的位置添加到 CMake 的搜索路径中，以便于 find_package() 找到它。
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm")
find_package(LLVM REQUIRED CONFIG)

# llvm 版本的判断
if(NOT "13" VERSION_EQUAL "${LLVM_VERSION_MAJOR}")
    message(FATAL_ERROR "LLVM version ${LLVM_VERSION_MAJOR} is not supported")
endif()

message(STATUS "Found LLVM ${LLVM_PACAKGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in ${LT_LLVM_INSTALL_DIR}")

message("LLVM STATUS:
    Definitions: ${LLVM_DEFINITIONS}
    Includes ${LLVM_INCLUDE_DIRS}
    Libraries ${LLVM_LIBRARY_DIRS}
    Target ${LLVM_TARGETS_TO_BUILD}
")

# 设置 llvm 头文件和库的搜索路径
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# 构建配置
# 和 llvm 使用相同的 c++ 标准
set(CMAKE_CXX_STANDARD 14 CACHE STRING "C++14 standard")

# 构建类型
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
endif()

# 编译器配置
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fdiagnostics-color=always")

# LLVM 通常是在没有 RTTI 的情况下构建的，需要保持一致
if(NOT LLVM_ENABLE_RTTI)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# -fvisibility-inlines-hidden 在构建 LLVM 的时候会被设置，在 Darwin 上也加上
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fvisibility-inlines-hidden" SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)
if(SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden")
endif()

# 设置构建目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

# 添加子模块
add_subdirectory(lib)
add_subdirectory(HelloWorld)
add_subdirectory(tools)