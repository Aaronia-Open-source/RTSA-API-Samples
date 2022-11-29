add_library(AARTSAAPIWrapper AARTSAAPIWrapper.cpp)

target_link_libraries(AARTSAAPIWrapper PUBLIC AaroniaRTSAAPI fmt::fmt)
target_include_directories(AARTSAAPIWrapper PUBLIC include)
