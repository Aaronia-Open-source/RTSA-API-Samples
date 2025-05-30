    include(FindPackageHandleStandardArgs)

    set(RTSAAPI_INCLUDE_DIR_FOUND FALSE)
    set(RTSAAPI_LIBRARY_FOUND FALSE)
    set(RTSAAPI_DLL_FOUND FALSE)


if(WIN32)
    if(AARONIA_SDK_DIRECTORY AND IS_DIRECTORY "${AARONIA_SDK_DIRECTORY}")
        find_path(RTSAAPI_INCLUDE_DIR NAMES aaroniartsaapi.h
            HINTS "${AARONIA_SDK_DIRECTORY}"
            NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
        )
        find_library(RTSAAPI_LIBRARY NAMES AaroniaRTSAAPI aaroniartsaapi
            HINTS "${AARONIA_SDK_DIRECTORY}"
            NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
        )
        find_file(RTSAAPI_DLL NAMES ${CPP_RTSAAPI_DLIB}
            HINTS "${AARONIA_SDK_DIRECTORY}"
            NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH
        )

        if(RTSAAPI_INCLUDE_DIR)
            set(RTSAAPI_INCLUDE_DIR_FOUND TRUE)
        endif()
        if(RTSAAPI_LIBRARY)
            set(RTSAAPI_LIBRARY_FOUND TRUE)
        endif()
        if(RTSAAPI_DLL)
            set(RTSAAPI_DLL_FOUND TRUE)
        endif()
    else()
        message(STATUS "AARONIA_SDK_DIRECTORY is not set or is not a valid directory: '${AARONIA_SDK_DIRECTORY}'")
    endif()

    find_package_handle_standard_args(RTSAAPI
        REQUIRED_VARS RTSAAPI_LIBRARY RTSAAPI_INCLUDE_DIR
        FAIL_MESSAGE "Aaronia RTSA API (aaroniartsaapi.h and AaroniaRTSAAPI.lib) not found in AARONIA_SDK_DIRECTORY: '${AARONIA_SDK_DIRECTORY}'. Please ensure AARONIA_SDK_DIRECTORY is set correctly in the main CMakeLists.txt and points to the directory containing these files."
    )

    mark_as_advanced(RTSAAPI_INCLUDE_DIR RTSAAPI_LIBRARY RTSAAPI_DLL AARONIA_SDK_DIRECTORY)

    if(RTSAAPI_FOUND)
        set(RTSAAPI_LIBRARIES ${RTSAAPI_LIBRARY})
        set(RTSAAPI_INCLUDE_DIRS ${RTSAAPI_INCLUDE_DIR})
        set(RTSAAPI_DEFINITIONS "")

        if(NOT TARGET Aaronia::RTSAAPI)
            if(RTSAAPI_DLL AND EXISTS "${RTSAAPI_DLL}")
                add_library(Aaronia::RTSAAPI SHARED IMPORTED GLOBAL)
                set_target_properties(Aaronia::RTSAAPI PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${RTSAAPI_INCLUDE_DIR}"
                    IMPORTED_IMPLIB "${RTSAAPI_LIBRARY}"
                    IMPORTED_LOCATION "${RTSAAPI_DLL}"
                )
                get_filename_component(RTSAAPI_DLL_DIR_COMPONENT "${RTSAAPI_DLL}" DIRECTORY)
                set(RTSAAPI_RUNTIME_LIBRARY_DIRS ${RTSAAPI_DLL_DIR_COMPONENT})
            else()
                add_library(Aaronia::RTSAAPI INTERFACE IMPORTED GLOBAL)
                set_target_properties(Aaronia::RTSAAPI PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${RTSAAPI_INCLUDE_DIR}"
                    INTERFACE_LINK_LIBRARIES "${RTSAAPI_LIBRARY}"
                )
            endif()
        endif()

        message(STATUS "RTSA API Search in: ${AARONIA_SDK_DIRECTORY}")
        message(STATUS "  Include Dir: ${RTSAAPI_INCLUDE_DIR} (Found: ${RTSAAPI_INCLUDE_DIR_FOUND})")
        message(STATUS "  Library:     ${RTSAAPI_LIBRARY} (Found: ${RTSAAPI_LIBRARY_FOUND})")
        if(RTSAAPI_DLL AND EXISTS "${RTSAAPI_DLL}")
            message(STATUS "  DLL:         ${RTSAAPI_DLL} (Found: ${RTSAAPI_DLL_FOUND})")
        else()
            message(STATUS "  DLL:         Not found in ${AARONIA_SDK_DIRECTORY} (Found: ${RTSAAPI_DLL_FOUND})")
        endif()
    else()
        message(STATUS "Aaronia RTSA API not found. Check AARONIA_SDK_DIRECTORY.")
    endif()
else()

    find_library(
        RTSAAPI_LIB
        AaroniaRTSAAPI
        HINTS ${RTSA_INSTALLATION_DIRECTORY}
        PATHS
        ${AARONIA_SDK_DIRECTORY}
        REQUIRED
        )

    # $ENV{HOME}/aaronia/rtsa/Debug/Aaronia-RTSA-Suite

    get_filename_component(RTSA_INSTALL_DIRECTORY "${RTSAAPI_LIB}" DIRECTORY)

    message(DEBUG "Found RTSA API in ${RTSAAPI_LIB}")
    message(DEBUG "RTSA installation directory: ${AARONIA_SDK_DIRECTORY}")

    add_library(Aaronia::RTSAAPI SHARED IMPORTED)
    set_target_properties(Aaronia::RTSAAPI PROPERTIES IMPORTED_LOCATION "${RTSAAPI_LIB}")
    target_include_directories(Aaronia::RTSAAPI INTERFACE "${AARONIA_SDK_DIRECTORY}/sdk")
endif()