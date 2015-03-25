add_subdirectory (modules)

include_directories (${PROJECT_SOURCE_DIR}/include)

add_library( hiredis SHARED IMPORTED )
set_property (TARGET hiredis PROPERTY IMPORTED_LOCATION /usr/local/lib/libhiredis.so)

add_executable (wiseup wiseup.cpp ipc.cpp clientHandler.cpp jsoncpp.cpp nrfTaskMng.cpp)
target_link_libraries (wiseup nrf24l01 mraa hiredis event ${CMAKE_THREAD_LIBS_INIT})
