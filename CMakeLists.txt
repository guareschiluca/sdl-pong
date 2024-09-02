cmake_minimum_required(VERSION 3.0)
project(build)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the CXX flags for Emscripten to support both PNG and JPG
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_SDL_IMAGE=2 -s USE_FREETYPE=1 -s USE_HARFBUZZ=1 -s SDL2_IMAGE_FORMATS=\"[\"png\",\"jpg\"]\" -s USE_SDL_MIXER=2 -s SDL2_MIXER_FORMATS=\"[\"mp3\"]\"")

# Add source files
file(GLOB_RECURSE SOURCES "SDL Pong/*.cpp" "SDL Pong/*.h")

# Preload files
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --use-preload-plugins --preload-file \"${CMAKE_SOURCE_DIR}\\res@/res\"")

# Add include directories
include_directories("SDL2/include" "SDL2/TTF/include" "SDL2/Image/include" "SDL2/Mixer/include")

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Set the initial memory and allow memory growth (just unoptimized resources, would be easy to solve in a real world scenario)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-sINITIAL_MEMORY=20971520 -sALLOW_MEMORY_GROWTH=1")

# Link SDL2, SDL2_ttf, SDL2_image, SDL2_mixer and dependencies for Emscripten
target_link_libraries(${PROJECT_NAME} SDL2 SDL2_ttf SDL2_image SDL2_mixer freetype harfbuzz z png)
