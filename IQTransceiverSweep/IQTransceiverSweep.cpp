#include "../helper.h"

void streamIQ(AARTSAAPI_Device d, AARTSAAPI_Config * centerConfig, AARTSAAPI_Config * demodConfig)
{
	static const double pi = 4.0 * atan(1.0);
	static const double	zeroDBm = sqrt(1.0 / 100.0);
	static const double	oneDBm = sqrt(1.0 / 10.0);
	static const ptrdiff_t	nsamples = 5000;

	float	iqbuffer[2 * nsamples];
	float	iqbuffer2[2 * nsamples];

	// Prepare buffer with 16k center tone
	for (int i = 0; i < nsamples; i++)
	{
		iqbuffer[2 * i + 0] = float(zeroDBm);
		iqbuffer[2 * i + 1] = 0.0;
		iqbuffer2[2 * i + 0] = float(oneDBm);
		iqbuffer2[2 * i + 1] = 0.0;
	}

	// Prepare output packet
	AARTSAAPI_Packet	opacket = { sizeof(AARTSAAPI_Packet) };

	// Frequency range

	opacket.size = 2;
	opacket.stride = 2;
	opacket.fp32 = iqbuffer;
	opacket.stepFrequency = 1.0e6;

	std::this_thread::sleep_for( std::chrono::milliseconds(500));

	// Clear input pipe

	int32_t num = 0;
	AARTSAAPI_AvailPackets(&d, 0, &num);
	AARTSAAPI_ConsumePackets(&d, 0, num);

	double	startTime;
	AARTSAAPI_GetMasterStreamTime(&d, startTime);

	// Send lead in packet to improve startup synch

	opacket.flags = AARTSAAPI_PACKET_STREAM_START;
	opacket.num = 0;
	opacket.startTime = opacket.endTime = startTime;
	opacket.startFrequency = 2.0e9;
	AARTSAAPI_SendPacket(&d, 0, &opacket);

	// Now for the real packets

	opacket.num = nsamples;
	opacket.flags = AARTSAAPI_PACKET_SEGMENT_START | AARTSAAPI_PACKET_SEGMENT_END;

	// Prepare input packet
	AARTSAAPI_Packet	ipacket = { sizeof(AARTSAAPI_Packet) };

	double	startFrequency = 1.0e9, endFrequency = 1.5e9, stepFrequency = 1.0e6;

	bool	odd = false;

	double	frequency = startFrequency;

	// First packet with a 100ms offset
	double	delay = 0.1;

	while (frequency < endFrequency)
	{
		// Offset the center frequency to avoid a DC clash
		double	centerFrequency = frequency + 5.0e6;

		AARTSAAPI_ConfigSetFloat(&d, centerConfig, centerFrequency);
		AARTSAAPI_ConfigSetFloat(&d, demodConfig, frequency);

		opacket.startFrequency = frequency - 0.5e6;

		double	streamTime;

		// Get the current system time

		AARTSAAPI_GetMasterStreamTime(&d, streamTime);

		// Prepare the packet to be played

		opacket.startTime = streamTime + delay;
		opacket.endTime = opacket.startTime + opacket.num / opacket.stepFrequency;

		// Send one packet

		if (odd)
			opacket.fp32 = iqbuffer2;
		else
			opacket.fp32 = iqbuffer;

		std::cout << "TX " << opacket.startTime - startTime << " .. " << opacket.endTime - startTime << " : " << frequency << " " << odd << std::endl;

		AARTSAAPI_SendPacket(&d, 0, &opacket);
		odd = !odd;

		// More packets with a 10ms offset
		delay = 0.01;

		// Pick center of pulse to look at
		double	midTime = 0.5 * (opacket.endTime + opacket.startTime);

		bool	found = false;
		while (!found)
		{
			AARTSAAPI_Result	res;
			while ((res = AARTSAAPI_GetPacket(&d, 0, 0, &ipacket)) == AARTSAAPI_EMPTY)
				std::this_thread::sleep_for( std::chrono::milliseconds(5));

			if (res == AARTSAAPI_OK)
			{
				std::cout << "RX " << ipacket.startTime - startTime << " .. " << ipacket.endTime - startTime << " : " << ipacket.startFrequency + 0.5 * ipacket.spanFrequency << " #" << ipacket.num;

				// Check for target time in packet
				if (ipacket.endTime >= midTime)
				{
					// Calculate sample offset
					ptrdiff_t	index = ptrdiff_t(floor((midTime - ipacket.startTime) * ipacket.stepFrequency));
					if (index >= 0 && index < ipacket.num)
						std::cout << " DATA " << log10(ipacket.fp32[2 * index] * ipacket.fp32[2 * index] + ipacket.fp32[2 * index + 1] * ipacket.fp32[2 * index + 1]) * 10 << std::endl;
					else
						std::cout << " FAIL " << index;

					found = true;
				}

				std::cout << std::endl;

				AARTSAAPI_ConsumePackets(&d, 0, 1);
			}
		}

		frequency += stepFrequency;
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

				if ((res = AARTSAAPI_EnumDevice(&h, L"spectranv6", 0, &dinfo)) == AARTSAAPI_OK)
				{
					AARTSAAPI_Device	d;

					// Try to open the first V6 in the system in transmitter mode

					if ((res = AARTSAAPI_OpenDevice(&h, &d, L"spectranv6/iqtransceiver", dinfo.serialNumber)) == AARTSAAPI_OK)
					{
						AARTSAAPI_Config	config, root, centerConfig, demodConfig;

						if (AARTSAAPI_ConfigRoot(&d, &root) == AARTSAAPI_OK)
						{
							// Select the first receiver channel

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/receiverchannel") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"Rx1");

							// Select the center frequency of the tuner

							if (AARTSAAPI_ConfigFind(&d, &root, &centerConfig, L"main/centerfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &centerConfig, 2440.0e6);

							// Select the frequency range of the tuner

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/spanfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 50.0e6);

							// Select the frequency range of the receiver demodulator to pick up a
							// frequency range from the input stream

							if (AARTSAAPI_ConfigFind(&d, &root, &demodConfig, L"main/demodcenterfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &demodConfig, 2430.5e6);

							// Select the frequency span of the receiver demodulator

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/demodspanfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 100.0e3);

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

									streamIQ(d, &centerConfig, &demodConfig);
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
