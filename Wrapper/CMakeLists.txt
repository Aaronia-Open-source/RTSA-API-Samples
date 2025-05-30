cmake_minimum_required(VERSION 3.16)
project(WrapperSample VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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


add_executable(${PROJECT_NAME} WrapperSample.cpp AaroniaRtsaSdkWrapper.cpp)

if(WIN32)
target_include_directories(${PROJECT_NAME} PRIVATE ${AARONIA_SDK_DIRECTORY})

else()
target_include_directories(${PROJECT_NAME} PRIVATE ${AARONIA_SDK_DIRECTORY}/sdk)
endif()

