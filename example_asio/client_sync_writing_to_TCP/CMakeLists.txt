
add_executable(server_test server.cpp)

add_executable(sync_write_some_to_TCP sync_write_some_to_TCP.cpp)
target_include_directories(sync_write_some_to_TCP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_link_libraries(sync_write_some_to_TCP PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)

add_executable(sync_readFromSocket sync_readFromSocket.cpp)
target_include_directories(sync_readFromSocket PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_link_libraries(sync_readFromSocket PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)
