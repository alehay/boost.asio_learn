cmake_minimum_required(VERSION 3.5)


project(Daemon-YandexS3)
add_compile_options( -pg -ggdb -O0 -pg)
#add_compile_options(-pedantic-errors)
# add_compile_options(-Wall)
 add_compile_options(-Wextra)
 add_compile_options(-Wfloat-equal) 
 add_compile_options( -Wpointer-arith)
 add_compile_options( -Wstrict-overflow=5) 

 set(CMAKE_CXX_STANDARD 20)
 set(CMAKE_CXX_STANDARD_REQUIRED ON)
 


add_subdirectory(libs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/boost)

add_executable(server http_server_sync_ssl.cpp)
add_executable(client http_client_sync_ssl.cpp)
add_executable(stream_test stream_test.cpp)
add_executable(simple_server_asio simple_server_asio.cpp)
add_executable(simple_server simple_unix_server.cpp)



target_include_directories(server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_include_directories(client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_include_directories(stream_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_include_directories(simple_server_asio PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)


target_link_libraries(server PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)
target_link_libraries(client PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)
target_link_libraries(stream_test PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)
target_link_libraries(simple_server_asio  PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)

add_subdirectory(client_sync_writing_to_TCP)
add_subdirectory(async)
add_subdirectory(chapter_client_server_impliment)