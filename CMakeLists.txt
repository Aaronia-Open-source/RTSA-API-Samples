cmake_minimum_required(VERSION 3.16)
project(RTSA_SDK_SAMPLES VERSION 1.0 LANGUAGES C CXX)

################################################################################################
#                                                                                              #
#                                    S E T U P                                                 #
#                                                                                              #
################################################################################################


if(WIN32)
# WINDOWS:

    # Set this paths according to installation location of your RTSA SUITE Pro
    set(AARONIA_RTSA_INSTALL_DIRECTORY "C:\\Program Files\\Aaronia AG\\Aaronia RTSA-Suite PRO") 

    # Set this paths according to installation location of your RTSA SUITE Pro, paths.xml location, default = AARONIA_RTSA_INSTALL_DIRECTORY
    set(AARONIA_XML_LOOKUP_DIRECTORY ${AARONIA_RTSA_INSTALL_DIRECTORY})
else()
# LINUX: 

    # Set this paths according to installation location of your RTSA SUITE Pro
    set(AARONIA_RTSA_INSTALL_DIRECTORY "/opt/aaronia-rtsa-suite/Aaronia-RTSA-Suite-PRO")
     # Set this paths according to installation location of your RTSA SUITE Pro, paths.xml location, default = AARONIA_RTSA_INSTALL_DIRECTORY
    set(AARONIA_XML_LOOKUP_DIRECTORY ${AARONIA_RTSA_INSTALL_DIRECTORY})
endif()


# no changes from here on
if(WIN32) 
    set(CPP_RTSAAPI_DLIB "AaroniaRTSAAPI.dll")   
    set(AARONIA_SDK_DIRECTORY "${AARONIA_RTSA_INSTALL_DIRECTORY}\\sdk" CACHE PATH "")

else()
    set(CPP_RTSAAPI_DLIB "libAaroniaRTSAAPI.so")
    set(AARONIA_SDK_DIRECTORY ${AARONIA_RTSA_INSTALL_DIRECTORY} CACHE PATH "")
endif()

if(NOT RTSA_BUILDAPP_INTERNAL_SDK)
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    find_package(RTSAAPI REQUIRED)
endif()

set(ESC_AARONIA_RTSA_INSTALL_DIRECTORY "${AARONIA_RTSA_INSTALL_DIRECTORY}")
set(ESC_AARONIA_XML_LOOKUP_DIRECTORY "${AARONIA_XML_LOOKUP_DIRECTORY}")
set(ESC_CPP_RTSAAPI_DLIB "${CPP_RTSAAPI_DLIB}")
set(ESC_AARONIA_SDK_DIRECTORY "${AARONIA_SDK_DIRECTORY}")

string(REPLACE "\\" "\\\\" ESC_AARONIA_RTSA_INSTALL_DIRECTORY "${AARONIA_RTSA_INSTALL_DIRECTORY}")
string(REPLACE "\\" "\\\\" ESC_AARONIA_XML_LOOKUP_DIRECTORY "${AARONIA_XML_LOOKUP_DIRECTORY}")
string(REPLACE "\\" "\\\\" ESC_CPP_RTSAAPI_DLIB "${CPP_RTSAAPI_DLIB}")
string(REPLACE "\\" "\\\\" ESC_AARONIA_SDK_DIRECTORY "${AARONIA_SDK_DIRECTORY}")
string(REPLACE "/" "\\\\" ESC_AARONIA_SDK_DIRECTORY "${AARONIA_SDK_DIRECTORY}")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/helper.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/helper.h
)

add_subdirectory(IQReceiver)
add_subdirectory(IQReceiverEco)
add_subdirectory(IQTransceiver)
add_subdirectory(IQTransceiverEco)
add_subdirectory(IQTransceiverLoopback)
add_subdirectory(IQTransceiverSweep)
add_subdirectory(IQTransmitter)
add_subdirectory(IQTransmitterEco)
add_subdirectory(RawIQ)
add_subdirectory(RawIQSampleRate)
add_subdirectory(RawIQ2RX)
add_subdirectory(RawIQ2RXInterleave)
add_subdirectory(RawSpectrum)
add_subdirectory(RawSpectrumEco)
add_subdirectory(SweepSpectrum)
add_subdirectory(SweepSpectrumEco)
add_subdirectory(SweepStressTest)
add_subdirectory(TransferRate)
add_subdirectory(ConfigTree)
add_subdirectory(EnumDevices)
add_subdirectory(GPSTime)