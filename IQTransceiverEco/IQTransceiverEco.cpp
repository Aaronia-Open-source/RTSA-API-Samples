#include "../helper.h"

#define TEST_INPUT	0

#define BSIZE	65536

void streamIQ(AARTSAAPI_Device d)
{
#if TEST_INPUT
	static const double pi = 4.0 * atan(1.0);
	static const double	zeroDBm = sqrt(1.0 / 20.0);
	float* iqbuffer = new float[2 * BSIZE];
	float* riqbuffer = new float[2 * BSIZE];

	// Prepare a small frequency sweep range

	double	w = 0;
	for (int i = 0; i < BSIZE; i++)
	{
		double	phi = (double(i) / BSIZE * 2 - 1) * pi;
		w += phi;

		iqbuffer[2 * i + 1] = float(cos(w) * zeroDBm);
		iqbuffer[2 * i + 0] = float(sin(w) * zeroDBm);

		riqbuffer[2 * i + 1] = float(cos(-w) * zeroDBm);
		riqbuffer[2 * i + 0] = float(sin(-w) * zeroDBm);
	}
#endif

	// Prepare output packet
	AARTSAAPI_Packet	packet = { sizeof(AARTSAAPI_Packet) };

	// Stream 1000 packets

	int NumPackets = 100000;

	for (int i = 0; i < NumPackets; i++)
	{
		AARTSAAPI_Result	res;

		// Get the next data packet, sleep for some milliseconds, if none
		// available yet.

		while ((res = AARTSAAPI_GetPacket(&d, 0, 0, &packet)) == AARTSAAPI_EMPTY)
			std::this_thread::sleep_for( std::chrono::milliseconds(5));

		// If we actually got a packet

		if (res == AARTSAAPI_OK)
		{
			// Get the current system time

			double	streamTime;
			AARTSAAPI_GetMasterStreamTime(&d, streamTime);

			// Check if we are still close to the capture stream

			if (packet.startTime > streamTime - 0.1)
			{
				// Push packet into future for buffering

				packet.startTime += 0.2;
				packet.endTime += 0.2;

				// New center frequency

				packet.startFrequency = 2460.0e6 - 0.5 * packet.stepFrequency;

#if TEST_INPUT
				packet.fp32 = iqbuffer;
#endif
				// Send packet to transmitter queue

				AARTSAAPI_SendPacket(&d, 0, &packet);
			}
		}

		// Consume the packet from the receiver queue

		AARTSAAPI_ConsumePackets(&d, 0, 1);
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

	// Initialize library for large memory usage

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
				AARTSAAPI_DeviceInfo	dinfo = { sizeof(AARTSAAPI_DeviceInfo) };

				// Get the serial number of the first V6 in the system

				if ((res = AARTSAAPI_EnumDevice(&h, L"spectranv6eco", 0, &dinfo)) == AARTSAAPI_OK)
				{
					AARTSAAPI_Device	d;

					// Try to open the first V6 in the system in transmitter mode

					if ((res = AARTSAAPI_OpenDevice(&h, &d, L"spectranv6eco/iqtransceiver", dinfo.serialNumber)) == AARTSAAPI_OK)
					{
						AARTSAAPI_Config	config, root;

						if (AARTSAAPI_ConfigRoot(&d, &root) == AARTSAAPI_OK)
						{
							// Select the first receiver channel

							// Select the center frequency of the tuner

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/centerfreqrx") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 2420.0e6);

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/centerfreqtx") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 2450.0e6);

							// Select the frequency range of the tuner

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/spanfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 50.0e6);

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/reflevel") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 0.0e6);

							// Select the frequency range of the receiver demodulator to pick up a
							// frequency range from the input stream

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/demodcenterfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 2410.0e6);

							// Select the frequency span of the receiver demodulator

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/demodspanfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 4.0e6);

							// Select the transmitter gain

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/transgain") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 0.0);

							// Connect to the physical device

							if ((res = AARTSAAPI_ConnectDevice(&d)) == AARTSAAPI_OK)
							{
								// Start the receiver

								if (AARTSAAPI_StartDevice(&d) == AARTSAAPI_OK)
								{
									// Wait for the transceiver running

									while (AARTSAAPI_GetDeviceState(&d) != AARTSAAPI_RUNNING)
									{
										std::wcout << ".";
										std::wcout.flush();
										std::this_thread::sleep_for( std::chrono::milliseconds(100));
									}
									std::wcout << std::endl;

									// Send data to the transceiver

									streamIQ(d);
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
