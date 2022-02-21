message( " -- boost --  ")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})


#add_library(boost_asio STATIC IMPORTED GLOBAL)
#set_target_properties(boost_asio PROPERTIES 
#    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/liblib-asio.a
#)

#add_library(boost_beast STATIC IMPORTED GLOBAL)
#set_target_properties(boost_beast PROPERTIES 
#    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/liblib-beast.a
#)

add_library(boost_json STATIC IMPORTED GLOBAL)
set_target_properties(boost_json PROPERTIES 
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libboost_json-mt-x64.a
)

add_library(libboost_program_options STATIC IMPORTED GLOBAL)
set_target_properties(libboost_program_options PROPERTIES 
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libboost_program_options.a
)