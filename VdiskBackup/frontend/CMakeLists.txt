set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Quick Svg Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Quick Svg Network)
FetchContent_GetProperties(FluentUI)
message(${FluentUI_SOURCE_DIR})

file(GLOB_RECURSE frontend_srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)
if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}_frontend
            MANUAL_FINALIZATION
            ${frontend_srcs}
    )
else ()
    add_executable(${PROJECT_NAME}_frontend
            ${frontend_srcs}
    )
endif ()
#复制程序运行所需要的动态库
if (WIN32)
    if (MSVC)
        if (CMAKE_SIZEOF_VOID_P EQUAL 4)
            file(GLOB_RECURSE 3RDPARTY_DLL_DIR ${FluentUI_SOURCE_DIR}/3rdparty/msvc/x86/*.dll)
        elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
            file(GLOB_RECURSE 3RDPARTY_DLL_DIR ${FluentUI_SOURCE_DIR}/3rdparty/msvc/x64/*.dll)
        endif ()
    elseif (MINGW)
        file(GLOB_RECURSE 3RDPARTY_DLL_DIR ${FluentUI_SOURCE_DIR}/3rdparty/mingw/*.dll)
    endif ()
    file(COPY ${3RDPARTY_DLL_DIR} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif ()

#复制FluentUI源码到运行目录下，用于脚手架生成
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/source/)
file(COPY ${FluentUI_SOURCE_DIR}/src/ DESTINATION ${APPLICATION_DIR_PATH}/source/)


#如果是静态库则需要手动注册插件，导入FluentUI.h头文件
target_include_directories(${PROJECT_NAME}_frontend PRIVATE
        ${FluentUI_SOURCE_DIR}/src
)
target_include_directories(${PROJECT_NAME}_frontend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(${PROJECT_NAME}_frontend PROPERTIES
        WIN32_EXECUTABLE TRUE
)

#Release也支持日志打印代码位置
target_compile_definitions(${PROJECT_NAME}_frontend
        PRIVATE
        QT_MESSAGELOGCONTEXT
)

#目标文件链接库
target_link_libraries(${PROJECT_NAME}_frontend PRIVATE
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Svg
        Qt${QT_VERSION_MAJOR}::Network
        fluentuiplugin
)