cmake_minimum_required(VERSION 3.16)
project(drift_picotron_sdl2 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

enable_testing()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

# headless regression test for core/*.hpp -- no SDL dependency, so this is
# the one target that will also build unchanged for a future ESP32-P4 host
# test environment.
add_executable(drift_test tests/test_render.cpp)
target_include_directories(drift_test PRIVATE core)
add_test(NAME render_test COMMAND drift_test)

# main desktop build: SDL2 only, no image-loading dependency needed at runtime.
add_executable(drift_sdl2 platform/sdl2/main_sdl2.cpp)
target_include_directories(drift_sdl2 PRIVATE core platform/sdl2 ${SDL2_INCLUDE_DIRS})
target_link_directories(drift_sdl2 PRIVATE ${SDL2_LIBRARY_DIRS})
target_link_libraries(drift_sdl2 PRIVATE ${SDL2_LIBRARIES})
target_compile_options(drift_sdl2 PRIVATE ${SDL2_CFLAGS_OTHER})

# title-screen-only demo (`make title`) -- pins app.proc_ to tlloop so it
# never hands off to the (still-stub) game loop.
add_executable(title_demo platform/sdl2/main_title.cpp)
target_include_directories(title_demo PRIVATE core platform/sdl2 ${SDL2_INCLUDE_DIRS})
target_link_directories(title_demo PRIVATE ${SDL2_LIBRARY_DIRS})
target_link_libraries(title_demo PRIVATE ${SDL2_LIBRARIES})
target_compile_options(title_demo PRIVATE ${SDL2_CFLAGS_OTHER})

# dev tool: plays exactly one extracted sfx track in a loop, LEFT/RIGHT to
# switch tracks -- for auditioning individual tracks/instruments in
# isolation while tuning core/audio.hpp's mixer.
add_executable(track_player platform/sdl2/track_player.cpp)
target_include_directories(track_player PRIVATE core platform/sdl2 ${SDL2_INCLUDE_DIRS})
target_link_directories(track_player PRIVATE ${SDL2_LIBRARY_DIRS})
target_link_libraries(track_player PRIVATE ${SDL2_LIBRARIES})
target_compile_options(track_player PRIVATE ${SDL2_CFLAGS_OTHER})

# dev tool: lays every character-sprite definition out on screen and dumps a
# PNG, to sanity-check pixel/palette conversion without eyeballing a live
# window. Needs SDL2_image only for PNG encoding.
pkg_check_modules(SDL2_IMAGE SDL2_image)
if(SDL2_IMAGE_FOUND)
    add_executable(sprite_viewer platform/sdl2/sprite_viewer.cpp)
    target_include_directories(sprite_viewer PRIVATE core platform/sdl2 ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
    target_link_directories(sprite_viewer PRIVATE ${SDL2_LIBRARY_DIRS} ${SDL2_IMAGE_LIBRARY_DIRS})
    target_link_libraries(sprite_viewer PRIVATE ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
    target_compile_options(sprite_viewer PRIVATE ${SDL2_CFLAGS_OTHER} ${SDL2_IMAGE_CFLAGS_OTHER})
else()
    message(STATUS "SDL2_image not found; skipping sprite_viewer dev tool")
endif()
