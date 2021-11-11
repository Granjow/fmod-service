cmake_minimum_required(VERSION 3.7)

project("FMOD Service")

set(CPU x86_64 CACHE STRING "CPU, one of [x86|x86_64|arm|armhf]")
set(CONFIG Debug CACHE STRING "Debug or Release")
set(FMOD_VERSION "20203")

set(FLAGS "")
set(SUFFIX "")

set(CMAKE_CXX_STANDARD 14)

if (${CPU} STREQUAL x86_64)
    message(STATUS "Compiling for x86_64")
    set(FLAGS "${FLAGS} -m64")
elseif (${CPU} STREQUAL x86)
    message(STATUS "Compiling for x86")
    set(FLAGS "${FLAGS} -m32")
elseif (${CPU} STREQUAL armhf)
    message(STATUS "Compiling for ARM with hard float")
    set(FLAGS "${FLAGS} -marm -march=armv6 -mfpu=vfp -mfloat-abi=hard")
elseif (${CPU} STREQUAL arm)
    message(STATUS "Compiling for ARM with soft float")
    set(FLAGS "${FLAGS} -marm -march=armv6 -mfpu=vfp -mfloat-abi=softfp")
else ()
    message(FATAL_ERROR "Unknown CPU: ${CPU}")
endif ()

if (${CONFIG} STREQUAL Debug)
    set(FLAGS "${FLAGS} -g")
    set(SUFFIX L)
else ()
    set(FLAGS "${FLAGS} -O2")
endif ()

set(FMOD "${CMAKE_SOURCE_DIR}/lib/fmodstudioapi${FMOD_VERSION}linux/api/")

if (NOT EXISTS ${FMOD})
    MESSAGE(STATUS "Extracting FMOD")
    execute_process(COMMAND
            tar "x" "-C" "${CMAKE_SOURCE_DIR}/lib" "-f" "${CMAKE_SOURCE_DIR}/resources/fmod-api/fmodstudioapi${FMOD_VERSION}linux.tar.gz"
            )
endif ()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CAVE-SOUND-SOURCES
        src/fmod-server.cc
        src/unix-server.cc
        src/server.cc
        ${FMOD}/studio/examples/common.cpp
        ${FMOD}/studio/examples/common_platform.cpp
        )
set(CAVE-SOUND-ZMQ-SOURCES
        src/CaveSoundZMQ.cpp
        src/SingleFmodEvent.cpp
        src/ContinuousFmodEvent.cpp
        src/FmodEvents.cpp
        src/unix-server.cc
        src/server.cc
        ${FMOD}/studio/examples/common.cpp
        ${FMOD}/studio/examples/common_platform.cpp
        )
set(FMOD-SERVICE-SOURCES
        src/fmod-controller.cpp
        src/FmodException.cpp
        src/ZmqApi.cpp
        ${FMOD}/studio/examples/common.cpp
        ${FMOD}/studio/examples/common_platform.cpp
        )
SET(FMOD_INCLUDES
        ${FMOD}/core/inc
        ${FMOD}/studio/inc
        ${FMOD}/studio/examples
        )
SET(FMOD_LIBRARIES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmodstudioapi${FMOD_VERSION}linux/api/core/lib/${CPU}/libfmod${SUFFIX}.so
        ${CMAKE_SOURCE_DIR}/lib/fmodstudioapi${FMOD_VERSION}linux/api/studio/lib/${CPU}/libfmodstudio${SUFFIX}.so
        )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")

include_directories(${FMOD_INCLUDES})

add_executable(cave-sound ${CAVE-SOUND-SOURCES})
target_include_directories(cave-sound PUBLIC ${FMOD}/core/inc)
target_link_libraries(cave-sound
        ${FMOD_LIBRARIES}
        Threads::Threads
        )

add_executable(tcp-receiver src/tcp-receiver.cpp lib/cppzmq/zmq.hpp)
target_link_libraries(tcp-receiver zmq)

add_executable(cave-sound-zmq ${CAVE-SOUND-ZMQ-SOURCES})
target_include_directories(cave-sound-zmq PUBLIC ${FMOD}/core/inc)
target_link_libraries(cave-sound-zmq
        ${FMOD_LIBRARIES}
        Threads::Threads
        zmq
        )

add_executable(fmod-service ${FMOD-SERVICE-SOURCES})
target_include_directories(fmod-service PUBLIC ${FMOD}/core/inc)
target_link_libraries(fmod-service
        ${FMOD_LIBRARIES}
        Threads::Threads
        zmq
        )
