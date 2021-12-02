
add_executable(asinc_write asinc_client_write.cpp)
target_include_directories(asinc_write PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_link_libraries(asinc_write PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)


add_executable(asinc_read asinc_client_read.cpp)
target_include_directories(asinc_read PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_link_libraries(asinc_read PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)

