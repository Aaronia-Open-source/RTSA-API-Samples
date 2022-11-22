#include <aaroniartsaapi.h>

#include <iostream>
#include <string>
#include <chrono>
#include <thread>

void measureTransfer(AARTSAAPI_Device d)
{

	for (int i = 0; i < 100; i++)
	{
		AARTSAAPI_Config	config, root;

		double	usb1 = 0, usb2 = 0, samples = 0;

		// Update health data

		if (AARTSAAPI_ConfigHealth(&d, &root) == AARTSAAPI_OK)
		{
			// Get main USB transfer rate

			if (AARTSAAPI_ConfigFind(&d, &root, &config, L"mainusbbytessecond") == AARTSAAPI_OK)
				AARTSAAPI_ConfigGetFloat(&d, &config, &usb1);

			// Get boost USB transfer rate

			if (AARTSAAPI_ConfigFind(&d, &root, &config, L"boostusbbytessecond") == AARTSAAPI_OK)
				AARTSAAPI_ConfigGetFloat(&d, &config, &usb2);

			// Get sample tramsfer rate

			if (AARTSAAPI_ConfigFind(&d, &root, &config, L"rx1iqsamplessecond") == AARTSAAPI_OK)
				AARTSAAPI_ConfigGetFloat(&d, &config, &samples);

			std::wcout << "Transfer " << i << " : " << usb1 << " + " << usb2 << " Samples: " << samples << std::endl;
		}

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}

int main()
{
	AARTSAAPI_Result	res;

	// Initialize library for medium memory usage

	if ((res = AARTSAAPI_Init(AARTSAAPI_MEMORY_MEDIUM)) == AARTSAAPI_OK)
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

					// Try to open the first V6 in the system in raw mode

					if ((res = AARTSAAPI_OpenDevice(&h, &d, L"spectranv6/raw", dinfo.serialNumber)) == AARTSAAPI_OK)
					{
						// Begin configuration, get root of configuration tree

						AARTSAAPI_Config	config, root;

						if (AARTSAAPI_ConfigRoot(&d, &root) == AARTSAAPI_OK)
						{
							// Select the first receiver channel

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/receiverchannel") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"Rx1");

							// Select iq as output format

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/outputformat") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"iq");

							// Use fast receiver clock

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/receiverclock") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"245MHz");

							// Connect to the physical device

							if ((res = AARTSAAPI_ConnectDevice(&d)) == AARTSAAPI_OK)
							{
								// Start the receiver

								if (AARTSAAPI_StartDevice(&d) == AARTSAAPI_OK)
								{
									// Receive some spectra

									measureTransfer(d);

									// Stop the receiver

									AARTSAAPI_StopDevice(&d);
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
