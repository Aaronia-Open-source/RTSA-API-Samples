find_library(
    RTSAAPI_LIB
    AaroniaRTSAAPI
    HINTS ${RTSA_INSTALLATION_DIRECTORY}
    PATHS
        $ENV{HOME}/Aaronia/RTSA/Aaronia-RTSA-Suite-PRO
        /opt/Aaronia/RTSA/Aaronia-RTSA-Suite-PRO
    REQUIRED
    )

# $ENV{HOME}/aaronia/rtsa/Debug/Aaronia-RTSA-Suite

get_filename_component(RTSA_INSTALL_DIRECTORY "${RTSAAPI_LIB}" DIRECTORY)

message(DEBUG "Found RTSA API in ${RTSAAPI_LIB}")
message(DEBUG "RTSA installation directory: ${RTSA_INSTALL_DIRECTORY}")

add_library(AaroniaRTSAAPI SHARED IMPORTED)
set_target_properties(AaroniaRTSAAPI PROPERTIES IMPORTED_LOCATION "${RTSAAPI_LIB}")
target_include_directories(AaroniaRTSAAPI INTERFACE "${RTSA_INSTALL_DIRECTORY}")
