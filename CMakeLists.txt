cmake_minimum_required(VERSION 3.16)
project(rtsaapi_samples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

if ((CMAKE_BUILD_TYPE STREQUAL "Debug") OR (CMAKE_BUILDTYPE STREQUAL "RelWithDebInfo"))
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        # using Clang
        set(CMAKE_CXX_FLAGS "-Weverything -march=native -mtune=native -mavx2 -Wno-missing-field-initializers")

    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # using GCC
        set(CMAKE_CXX_FLAGS "-Wall -Wextra -march=native -mtune=native -mavx2 -Wno-missing-field-initializers")

    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        # using Intel C++ (TODO)

    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # using Visual Studio C++ (TODO)

    endif()
endif()

find_package(RTSAAPI REQUIRED)
find_package(fmt REQUIRED)

add_subdirectory(AARTSAAPIWrapper)

add_subdirectory(ConfigTree)
add_subdirectory(EnumDevices)
add_subdirectory(GPSTime)
add_subdirectory(IQReceiver)
add_subdirectory(IQTransceiver)
add_subdirectory(IQTransmitter)
add_subdirectory(RawIQ)
add_subdirectory(RawIQ2RX)
add_subdirectory(RawIQSampleRate)
add_subdirectory(RawSpectrum)
add_subdirectory(SweepSpectrum)
add_subdirectory(SweepStressTest)
add_subdirectory(TransferRate)


