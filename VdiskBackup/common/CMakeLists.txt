file(GLOB_RECURSE common_srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)
message(${common_srcs})
add_library(${PROJECT_NAME}_common STATIC ${common_srcs})
target_include_directories(${PROJECT_NAME}_common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME}_common PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(${PROJECT_NAME}_common PRIVATE spdlog::spdlog_header_only)
target_link_libraries(${PROJECT_NAME}_common PRIVATE indicators::indicators)
target_link_libraries(${PROJECT_NAME}_common PRIVATE ftxui::dom ftxui::screen ftxui::component)
