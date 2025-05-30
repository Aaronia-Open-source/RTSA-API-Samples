#include "../helper.h"



void streamSpectra(AARTSAAPI_Device d)
{
	// ASCII art brightness levels

	static const wchar_t* hlevels = L"$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/|()1{}[]?-_+~<>i!lI;:,\"^`'. ";

	int decim = 0;

	// Test 1k spectra packets

	for (int i = 0; i < 1000000; i++)
	{
		// Prepare data packet

		AARTSAAPI_Packet	packet = { sizeof(AARTSAAPI_Packet) };
		AARTSAAPI_Result	res;

		// Get the next data packet, sleep for some milliseconds, if none
		// available yet.  We use channel 2, which is the Rx1 spectra output

		while ((res = AARTSAAPI_GetPacket(&d, 0, 0, &packet)) == AARTSAAPI_EMPTY)
			std::this_thread::sleep_for( std::chrono::milliseconds(5));

		// If we actually got a packet

		if (res == AARTSAAPI_OK)
		{
			const float* fp = packet.fp32;

			for (int s = 0; s < packet.num; s++)
			{
				decim++;
				if (decim == 100)
				{
					wchar_t	buff[129];

					int	k = 0;
					for (int j = 0; j < 128; j++)
					{
						int	l = int(packet.size) * (j + 1) / 128;
						float	mv = -200.0;
						for (int n = k; n < l; n++)
							if (fp[n] > mv)
								mv = fp[n];
						k = l;

						int	 mi = -int(mv + 10);
						if (mi >= 0 && mi < 69)
							buff[j] = hlevels[mi];
						else
							buff[j] = L'_';
					}

					buff[128] = 0;
					std::wcout << buff << std::endl;
					decim = 0;
				}

				// Advance to next sample

				fp += packet.stride;
			}

			// Remove the first packet from the packet queue

			AARTSAAPI_ConsumePackets(&d, 0, 1);
		}
		else
			break;
	}

}

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

				if ((res = AARTSAAPI_EnumDevice(&h, L"spectranv6eco", 0, &dinfo)) == AARTSAAPI_OK)
				{
					AARTSAAPI_Device	d;

					// Try to open the first V6 in the system in raw mode

					if ((res = AARTSAAPI_OpenDevice(&h, &d, L"spectranv6eco/rtsa", dinfo.serialNumber)) == AARTSAAPI_OK)
					{
						// Begin configuration, get root of configuration tree

						AARTSAAPI_Config	config, root;

						if (AARTSAAPI_ConfigRoot(&d, &root) == AARTSAAPI_OK)
						{
							// Select the first receiver channel

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/centerfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 2420.0e6);

							// Set required span frequency to 64kHz

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/spanfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 40.0e6);

							// Set required rbw frequency to 100kHz

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/rbwfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 100.0e3);

							// Set the reference level of the receiver

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/reflevel") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, -20.0);

							// Connect to the physical device

							if ((res = AARTSAAPI_ConnectDevice(&d)) == AARTSAAPI_OK)
							{
								// Start the receiver

								if (AARTSAAPI_StartDevice(&d) == AARTSAAPI_OK)
								{
									// Receive some spectra

									streamSpectra(d);

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
