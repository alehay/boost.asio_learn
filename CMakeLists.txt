cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0079 NEW)

project(Boost_asio_test)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options( -pg -ggdb -O0 -pg)
#add_compile_options(-pedantic-errors)
# add_compile_options(-Wall)
add_compile_options(-Wextra)

add_subdirectory(libs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/boost)




# load upload file https 
add_subdirectory(./example_asio)
target_include_directories(HttpsServer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_include_directories(HttpsClient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)

target_link_libraries(HttpsServer PUBLIC boost_json pthread ssl crypto)
target_link_libraries(HttpsClient PUBLIC boost_json pthread ssl crypto)


add_subdirectory(example_json)
target_include_directories(jsonParceExample PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_link_libraries(jsonParceExample PUBLIC boost_json pthread)