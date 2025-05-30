#include "../helper.h"

int main()
{
	if (LoadRTSAAPI_with_searchpath() != 0)
	{
		std::wcerr << "Load RTSSAPI failed";
		return - 1; 
	}

	AARTSAAPI_Result	res;

	// Initialize library for medium memory usage

	if ((res = AARTSAAPI_Init_With_Path(AARTSAAPI_MEMORY_MEDIUM, CFG_AARONIA_XML_LOOKUP_DIRECTORY)) == AARTSAAPI_OK)
	{

		// Open a library handle for use by this application

		AARTSAAPI_Handle	h;

		if ((res = AARTSAAPI_Open(&h)) == AARTSAAPI_OK)
		{
			// Rescan all devices controlled by the aaronia library and update
			// the firmware if required.

			if ((res = AARTSAAPI_RescanDevices(&h, 2000)) == AARTSAAPI_OK)
			{
				// Get the serial number of the first V6 in the system

				AARTSAAPI_DeviceInfo	dinfo = { sizeof(AARTSAAPI_DeviceInfo) };

				if ((res = AARTSAAPI_EnumDevice(&h, L"spectranv6", 0, &dinfo)) == AARTSAAPI_OK)
				{
					AARTSAAPI_Device	d;

					std::wcerr << "AARTSAAPI_OpenDevice : " << dinfo.serialNumber << std::endl;

					// Try to open the first V6 in the system in raw mode

					if ((res = AARTSAAPI_OpenDevice(&h, &d, L"spectranv6/raw", dinfo.serialNumber)) == AARTSAAPI_OK)
					{
						// Begin configuration, get root of configuration tree

						AARTSAAPI_Config	root, config, health;

						if (AARTSAAPI_ConfigRoot(&d, &root) == AARTSAAPI_OK)
						{
							// Enable GPS Location and Time acquisition

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/gpsmode") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"Location and Time");

							// Select GPS time as the main source for the stream timing

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/sclksource") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"GPS Provider");

							// Connect to the device

							if ((res = AARTSAAPI_ConnectDevice(&d)) == AARTSAAPI_OK)
							{
								// Continuously poll the GPS state

								for (int i = 0; i < 3600; i++)
								{
									if (AARTSAAPI_ConfigHealth(&d, &health) == AARTSAAPI_OK)
									{
										int64_t	v;

										bool	gpsTimeValid = false;
										int		gpsNumSats = 0;
										double	gpsTime = 0, gpsTimeOffset = 0;

										// Number of visible GPS satellites

										if (AARTSAAPI_ConfigFind(&d, &health, &config, L"gpssats") == AARTSAAPI_OK)
										{
											AARTSAAPI_ConfigGetInteger(&d, &config, &v); gpsNumSats = v;
										}

										// Time stamp provided by GPS is valid and second tick is precise

										if (AARTSAAPI_ConfigFind(&d, &health, &config, L"gpstimevalid") == AARTSAAPI_OK)
										{
											AARTSAAPI_ConfigGetInteger(&d, &config, &v); gpsTimeValid = v != 0;
										}

										// Current GPS time in seconds since the start of the epoch

										if (AARTSAAPI_ConfigFind(&d, &health, &config, L"gpstime") == AARTSAAPI_OK)
											AARTSAAPI_ConfigGetFloat(&d, &config, &gpsTime);

										// Time offset between GPS time and stream time

										if (AARTSAAPI_ConfigFind(&d, &health, &config, L"gpstimeoffset") == AARTSAAPI_OK)
											AARTSAAPI_ConfigGetFloat(&d, &config, &gpsTimeOffset);

										std::wcout << "GPS " << gpsNumSats << " Sats " << gpsTimeValid << " Valid " << gpsTime << ", " << gpsTimeOffset << std::endl;
									}

									std::this_thread::sleep_for( std::chrono::milliseconds(1000));
								}

								// Release the hardware

								AARTSAAPI_DisconnectDevice(&d);
							}
							else
								std::wcerr << "AARTSAAPI_ConnectDevice failed : " << std::hex << res << std::endl;
						}

						// Close the device handle

						AARTSAAPI_CloseDevice(&h, &d);
					}
					else
						std::wcerr << "AARTSAAPI_OpenDevice failed : " << std::hex << res << std::endl;
				}
				else
					std::wcerr << "AARTSAAPI_EnumDevice failed : " << std::hex << res << std::endl;
			}
			else
				std::wcerr << "AARTSAAPI_RescanDevices failed : " << std::hex << res << std::endl;

			// Close the library handle

			AARTSAAPI_Close(&h);
		}
		else
			std::wcerr << "AARTSAAPI_Open failed : " << std::hex << res << std::endl;

		// Shutdown library, release resources

		AARTSAAPI_Shutdown();
	}
	else
		std::wcerr << "AARTSAAPI_Init failed : " << std::hex << res << std::endl;

	return 0;
}
