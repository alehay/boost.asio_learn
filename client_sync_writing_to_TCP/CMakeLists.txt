
add_executable(server_test server.cpp)

add_executable(sync_writing_to_TCP sync_writing_to_TCP.cpp)
target_include_directories(sync_writing_to_TCP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_link_libraries(sync_writing_to_TCP PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)
