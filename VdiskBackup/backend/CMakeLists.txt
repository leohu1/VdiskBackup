file(GLOB_RECURSE backend_srcs CONFIGURE_DEPENDS src/*.cpp include/*.h)
add_executable(${PROJECT_NAME}_backend ${backend_srcs})
target_include_directories(${PROJECT_NAME}_backend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


set_target_properties(${PROJECT_NAME}_backend PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_link_libraries(${PROJECT_NAME}_backend PRIVATE clipp::clipp)
target_link_libraries(${PROJECT_NAME}_backend PRIVATE yaml-cpp::yaml-cpp)
target_link_libraries(${PROJECT_NAME}_backend PRIVATE spdlog::spdlog_header_only)
target_link_libraries(${PROJECT_NAME}_backend PRIVATE indicators::indicators)
target_link_libraries(${PROJECT_NAME}_backend PRIVATE ftxui::dom ftxui::screen ftxui::component)
target_link_libraries(${PROJECT_NAME}_backend PRIVATE ${PROJECT_NAME}_common)

target_link_options(${PROJECT_NAME}_backend PRIVATE "/manifest" "/level='requireAdministrator'" "/uiAccess='false'" "/activeCodePage='UTF-8'")
