set(YAMC_SERVER_SOURCE_FILES
    main.cpp
    Server.cpp
    ../Sockets.cpp
)

set(YAMC_SERVER_HEADER_FILES
    Server.h
)

add_executable(yamc_server ${YAMC_SERVER_SOURCE_FILES} ${YAMC_SERVER_HEADER_FILES})
target_link_libraries(yamc_server PUBLIC ${SOCKET_LIBRARIES})
set_target_properties(yamc_server PROPERTIES 
                      VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                      CXX_STANDARD 17
)