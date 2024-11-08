cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")

add_definitions("-DVFS_FRONTEND -DHAVE_STRL")

set(OBOE_DIR oboe)
add_subdirectory(${OBOE_DIR} oboe)
include_directories(${OBOE_DIR}/include)
include_directories(${OBOE_DIR}/src)

include_directories("libretro-common/include")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(LIBRETRO_COMMON
        libretro-common/vfs/vfs_implementation.c
        libretro-common/string/stdstring.c
        libretro-common/encodings/encoding_utf.c
        libretro-common/file/file_path.c
        libretro-common/time/rtime.c
)

add_library(RetroRunner SHARED
        retro_runner.cpp

        core.cpp
        environment.cpp
        app.cpp
        input.cpp
        video.cpp

        video/video_gl.cpp
        video/hw_frame_buffer_gl.cpp
        video/software_texture_buffer.cpp
        video/shader_pass_gl.cpp

        input/software_input.cpp

        utils/threadsafe_queue.hpp
        utils/jnistring.cpp
        utils/utils.cpp

        ${LIBRETRO_COMMON}
)

# add lib dependencies
target_link_libraries(RetroRunner
        android
        log
        EGL
        oboe
        GLESv2
        GLESv3
)

