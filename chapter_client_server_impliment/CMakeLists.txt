# exampl_async_client_TCP
# exampl_sunc_client_UPD
# exampl_sync_client_TCP



add_executable(exampl_async_client_TCP exampl_async_client_TCP.cpp)
target_include_directories(exampl_async_client_TCP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_link_libraries(exampl_async_client_TCP PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)

add_executable(exampl_async_server_TCP exampl_async_server_TCP.cpp)
target_include_directories(exampl_async_server_TCP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_link_libraries(exampl_async_server_TCP PUBLIC boost_asio boost_beast boost_json pthread ssl crypto)

