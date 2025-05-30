cmake_minimum_required(VERSION 3.15)

project(IQTransceiverLoopback LANGUAGES CXX)

add_executable(${PROJECT_NAME} IQTransceiverLoopback.cpp "../helper.cpp")

if(NOT RTSA_BUILDAPP_INTERNAL_SDK)
        target_link_libraries(${PROJECT_NAME} PRIVATE Aaronia::RTSAAPI)

        if(WIN32)
            target_link_libraries(${PROJECT_NAME} PRIVATE DelayImp.lib)
            target_link_options(${PROJECT_NAME} PRIVATE "/DELAYLOAD:AaroniaRTSAAPI.dll")
        endif()
else() 
    target_link_libraries(${PROJECT_NAME} PRIVATE AaroniaRTSAAPI)
    target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../../Applications/AaroniaRTSAAPI")
endif()