set(YAMC_SOURCE_FILES
    gl3w.cpp
    main.cpp
    Chunk.cpp
    Shader.cpp
    GameView.cpp
    Camera.cpp
    Texture.cpp
    Terrain.cpp
    Renderer.cpp
    Mesh.cpp
    Game.cpp
    Inventory.cpp
    MeshBuilder.cpp
    Application.cpp
    View.cpp
    Entities.cpp
    SinglePlayerGame.cpp
    MultiPlayerGame.cpp
    WorldDataManager.cpp
    Sockets.cpp
    Network.cpp
)

set(YAMC_HEADER_FILES
    Chunk.h
    Shader.h
    GameView.h
    Camera.h
    Texture.h
    Terrain.h
    Renderer.h
    Mesh.h
    Game.h
    Inventory.h
    MeshBuilder.h
    Application.h
    View.h
    Entities.h
    SinglePlayerGame.h
    MultiPlayerGame.h
    WorldDataManager.h
    Sockets.h
    Network.h
)

set(YAMC_SHADERS
    ../data/shaders/default.vert
    ../data/shaders/default.frag
    ../data/shaders/ui.vert
    ../data/shaders/ui.frag
    ../data/shaders/color.vert
    ../data/shaders/color.frag
)

source_group("Shaders" FILES ${YAMC_SHADERS})

if(WIN32)
    set(SOCKET_LIBRARIES Ws2_32)
else()
    set(SOCKET_LIBRARIES)
endif()

add_executable(yamc ${YAMC_SOURCE_FILES} ${YAMC_HEADER_FILES} ${YAMC_SHADERS})
target_link_libraries(yamc PUBLIC glfw ${OPENGL_LIBRARIES} ${SOCKET_LIBRARIES})
set_target_properties(yamc PROPERTIES 
                      VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                      CXX_STANDARD 17
)

add_subdirectory(server)