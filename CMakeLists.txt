cmake_minimum_required(VERSION 3.24)
project(rtsaapi_samples)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

find_package(RTSAAPI)

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


