#ifndef HELPER_H
#define HELPER_H

#include <aaroniartsaapi.h>
#include <thread>
#include <iostream>
#include <string>
#include <cmath>

#ifdef _WIN32
#include <windows.h> 
#include <delayimp.h>
#include <strsafe.h>
#include <shlobj_core.h>
#endif

int LoadRTSAAPI_with_searchpath();

#define CFG_AARONIA_RTSA_INSTALL_DIRECTORY L"/opt/aaronia-rtsa-suite/Aaronia-RTSA-Suite-PRO"
#define CFG_AARONIA_XML_LOOKUP_DIRECTORY L"/opt/aaronia-rtsa-suite/Aaronia-RTSA-Suite-PRO"
#define CFG_CPP_RTSAAPI_DLIB L"libAaroniaRTSAAPI.so"
#define CFG_AARONIA_SDK_DIRECTORY L"\\opt\\aaronia-rtsa-suite\\Aaronia-RTSA-Suite-PRO"

#endif

