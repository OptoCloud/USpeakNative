cmake_minimum_required (VERSION 3.5)

set (project USpeakNative)
project (${project} LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_POSITION_INDEPENDENT_CODE ON)

if (${CMAKE_BUILD_TYPE} MATCHES "Release")
    set (CMAKE_CXX_FLAGS " /MD /DEBUG:NONE /O2 /Ob2")
else ()
    set (CMAKE_CXX_FLAGS " /MDd /DEBUG:FULL /Od /Ob0 /Wall")
endif ()

add_subdirectory(fmt)
add_subdirectory(libnyquist)
find_package (Opus REQUIRED CONFIG)

set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

add_library(${project} SHARED
    uspeaklite.cpp
    uspeaklite.h
    uspeaklitewrapper.cpp
    uspeakframecontainer.cpp
    uspeakframecontainer.h
    uspeakvolume.cpp
    uspeakvolume.h
    uspeakresampler.cpp
    uspeakresampler.h
    opuscodec/opuscodec.h
    opuscodec/opuscodec.cpp
    opuscodec/opusdelay.h
    opuscodec/opuserror.h
    opuscodec/bandmode.h
    opuscodec/bitrates.h
    opuscodec/opusapp.h
    internal/scopedspinlock.h
    internal/scopedtrylock.h
)

target_include_directories(${project} PRIVATE
    ${FMT_INCLUDE_DIRS}
    ${OPUS_INCLUDE_DIRS}
    ${LIBNYQUIST_INCLUDE_DIRS}
)

target_link_libraries(${project} PRIVATE
    ${CMAKE_DL_LIBS}
    Threads::Threads
    fmt::fmt
    libnyquist
)
