set(PLUGINS
    OpcodeCounter
    InjectFuncCall
)

set(OpcodeCounter_SOURCES
    OpcodeCounter.cpp
)
set(InjectFuncCall_SOURCES
    InjectFuncCall.cpp
)

# 设置编译器的配置
foreach( plugin ${PLUGINS})

    # 创建一个库给每个插件
    add_library(
        ${plugin}
        SHARED
        ${${plugin}_SOURCES}
    )

    # 为插件配置 include 目录
    target_include_directories(
        ${plugin}
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    )

    # 在 Dawwin 里，引用 LLVM 共享库中的符号，共享对象定义之前符号是未定义的符号，直到这些共享对象被加载到内存中构建会失败，提示 Undefined symbols for architecture x86_64。各种符号都是未定义的，这些符号后来会在运行时加载，所以是误报。这种情况可以通过 -undefined dynamic_lookup 来解决。
    target_link_libraries(
        ${plugin}
        "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>"
    )

endforeach()