find_package(spdlog CONFIG REQUIRED)
find_package(indicators CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(ftxui CONFIG REQUIRED)
find_package(clipp CONFIG REQUIRED)

add_subdirectory(common)
add_subdirectory(backend)
add_subdirectory(frontend)
