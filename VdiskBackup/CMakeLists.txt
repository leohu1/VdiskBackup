find_package(spdlog CONFIG REQUIRED)
find_package(indicators CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(ftxui CONFIG REQUIRED)
find_package(clipp CONFIG REQUIRED)

include(FetchContent)
FetchContent_Declare(
        FluentUI
        GIT_REPOSITORY https://github.com/zhuzichu520/FluentUI.git
        GIT_TAG 1.7.7
)

set(FLUENTUI_BUILD_EXAMPLES OFF)
set(FLUENTUI_BUILD_STATIC_LIB ON)

FetchContent_MakeAvailable(FluentUI)


add_subdirectory(common)
add_subdirectory(backend)
add_subdirectory(frontend)
