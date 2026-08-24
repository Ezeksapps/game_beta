cmake_minimum_required (VERSION 4.0)

project (game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "-O1")

# dependencies
set(DILIGENT_NO_ARCHIVER ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_HLSL ON CACHE BOOL "" FORCE)
add_subdirectory(deps/DiligentCore)

add_subdirectory(deps/DiligentTools)

set(RATS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(RATS_BUILD_CLIENT OFF CACHE BOOL "" FORCE)
set(RATS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RATS_STATIC_LIBRARY OFF CACHE BOOL "" FORCE)
set(RATS_SHARED_LIBRARY ON CACHE BOOL "" FORCE)
add_subdirectory(deps/librats)

add_subdirectory(deps/glfw)
add_subdirectory(deps/glm)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE SRC
    CONFIGURE_DEPENDS
    "src/*.cpp"
    "src/*.c"
)

add_executable(game ${SRC})

target_link_libraries(game
PRIVATE
    Diligent-GraphicsEngineOpenGL-shared
    Diligent-GraphicsEngineVk-shared
    Diligent-AssetLoader   # glTF loader
    Diligent-TextureLoader
    rats
    glfw
    glm
)
