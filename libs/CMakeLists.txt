# linking a static library
add_library(MySqlClient  STATIC IMPORTED GLOBAL )
set_target_properties(MySqlClient PROPERTIES 
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/mySqlClient/libmysqlclient.a
)
#dependencies
target_link_libraries( MySqlClient INTERFACE pthread z m rt dl ssl crypto  resolv)

add_subdirectory(boost)
