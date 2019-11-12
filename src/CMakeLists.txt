set(YAMC_SOURCE_FILES
    gl3w.cpp
    main.cpp
    Chunk.cpp
    Shader.cpp
    Game.cpp
    Camera.cpp
    Texture.cpp
    Terrain.cpp
    Renderer.cpp
    Mesh.cpp
)

set(YAMC_HEADER_FILES
    Chunk.h
    Shader.h
    Game.h
    Camera.h
    Texture.h
    Terrain.h
    Renderer.h
    Mesh.h
)

set(YAMC_SHADERS
    ../data/shaders/default.vert
    ../data/shaders/default.frag
    ../data/shaders/font.vert
    ../data/shaders/font.frag
    ../data/shaders/color.vert
    ../data/shaders/color.frag
)

source_group("Shaders" FILES ${YAMC_SHADERS})

add_executable(yamc ${YAMC_SOURCE_FILES} ${YAMC_HEADER_FILES} ${YAMC_SHADERS})
target_link_libraries(yamc PUBLIC glfw ${OPENGL_LIBRARIES})
set_target_properties(yamc PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")