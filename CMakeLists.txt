cmake_minimum_required (VERSION 4.0)

project (game_d)

# CHECK: Why are some libs coming out excessively large?
# CHECK: Exactly how does the executable want to link to shared obj libs

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# dependencies
set(DILIGENT_NO_ARCHIVER ON CACHE BOOL "" FORCE)
set(DILIGENT_NO_HLSL ON CACHE BOOL "" FORCE)
add_subdirectory(deps/DiligentCore)

add_subdirectory(deps/DiligentTools)

# CHECK: Archiver and super resolution probably not needed, also disable HLSL functionality and converters

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

add_executable(game_d ${SRC})

target_link_libraries(game_d
PRIVATE
    Diligent-GraphicsEngineOpenGL-shared
    Diligent-GraphicsEngineVk-shared
    Diligent-AssetLoader   # glTF loader
    Diligent-TextureLoader
    rats
    glfw
    glm
)
