cmake_minimum_required(VERSION 3.5)


project(Daemon-YandexS3)

add_subdirectory(libs)



add_executable(server http_server_sync_ssl.cpp)
add_executable(client http_client_sync_ssl.cpp)
add_executable(stream_test stream_test.cpp)


target_include_directories(server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_include_directories(client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_include_directories(stream_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)



target_link_libraries(server PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)
target_link_libraries(client PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)

target_link_libraries(stream_test PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)