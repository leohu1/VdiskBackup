cmake_minimum_required(VERSION 3.27)
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /manifest /level='requireAdministrator' /uiAccess='false' /activeCodePage='UTF-8'")
project(VdiskBackup)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

find_package(spdlog CONFIG REQUIRED)
find_package(indicators CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)
find_package(ftxui CONFIG REQUIRED)

add_executable(${PROJECT_NAME} main.cpp
        FileSystem.cpp
        FileSystem.h
        VolumeSystem.cpp
        VolumeSystem.h
        VdiskBackupManager.cpp
        VdiskBackupManager.h
        VirtDiskSystem.h
        VirtDiskSystem.cpp
        md5.hpp
        utils.h
        utils.cpp
        thread_pool.cpp
        thread_pool.h
        BackupConfigManager.cpp
        BackupConfigManager.h
)

set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog_header_only)
target_link_libraries(${PROJECT_NAME} PRIVATE indicators::indicators)
target_link_libraries(${PROJECT_NAME} PRIVATE cxxopts::cxxopts)
target_link_libraries(${PROJECT_NAME} PRIVATE ftxui::dom ftxui::screen ftxui::component)

set(SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/SetBoot.bat")
set(DESTINATION_PATH "${CMAKE_CURRENT_BINARY_DIR}")

# 使用file命令复制文件
add_custom_command(TARGET your_target
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_FILE} ${DESTINATION_PATH})

