cmake_minimum_required(VERSION 3.27)
project(VdiskBackup)

set(CMAKE_CXX_STANDARD 20)

set(PROJECT_REQUIRED_CONF ${PROJECT_SOURCE_DIR}/log.conf ${PROJECT_SOURCE_DIR}/CopySettings.yaml)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

if ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
    message(Debugging)
    list(APPEND PROJECT_REQUIRED_DLL ${PROJECT_SOURCE_DIR}/log4cplus/bin/log4cplusUD.dll)
else ("${CMAKE_BUILD_TYPE}" STREQUAL Releast)
    list(APPEND PROJECT_REQUIRED_DLL ${PROJECT_SOURCE_DIR}/log4cplus/bin/log4cplusU.dll)
endif ()

add_executable(${PROJECT_NAME} main.cpp
        FileSystem.cpp
        FileSystem.h
        VolumeSystem.cpp
        VolumeSystem.h
        VdiskBackupManager.cpp
        VdiskBackupManager.h
        VdiskBackupManager.h
        indicators.h
)

set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(common_util)

include(FetchContent)
FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG 0.8.0 # Can be a tag (yaml-cpp-x.x.x), a commit hash, or a branch name (master)
)
FetchContent_MakeAvailable(yaml-cpp)

set(spdlog_DIR ${PROJECT_SOURCE_DIR}/spdlog/lib/cmake/spdlog)
find_package(spdlog REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PUBLIC common_util yaml-cpp)

foreach (REQUIRED_DLL ${PROJECT_REQUIRED_DLL})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${REQUIRED_DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    message(STATUS "COPY ST_DLL ${REQUIRED_DLL}")
endforeach (REQUIRED_DLL)

foreach (REQUIRED_CONF ${PROJECT_REQUIRED_CONF})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${REQUIRED_CONF}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    message(STATUS "COPY CONF ${REQUIRED_CONF}")
endforeach (REQUIRED_CONF)

