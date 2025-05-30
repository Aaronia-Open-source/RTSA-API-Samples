#include "../helper.h"


void streamIQ(AARTSAAPI_Device d)
{
	static const double pi = 4.0 * atan(1.0);
	static const double	zeroDBm = sqrt(1.0 / 20.0);
	float	iqbuffer[32768];

	// Prepare buffer with alternating 8k tone and 8k silence
	for (int i = 0; i < 16384; i++)
	{
		double	phi = (double(i) / 32 * 2 - 1) * pi;

		if (i < 8192)
		{
			iqbuffer[2 * i + 1] = float(cos(phi) * zeroDBm);
			iqbuffer[2 * i + 0] = float(sin(phi) * zeroDBm);
		}
		else
		{
			iqbuffer[2 * i + 1] = 0.0;
			iqbuffer[2 * i + 0] = 0.0;
		}
	}

	// Prepare output packet
	AARTSAAPI_Packet	opacket = { sizeof(AARTSAAPI_Packet) };

	// Frequency range

	opacket.startFrequency = 2430.0e6;
	opacket.stepFrequency = 1.5e6;

	double	streamTime, startTime;

	// Get the current system time

	AARTSAAPI_GetMasterStreamTime(&d, streamTime);

	// Prepare the first packet to be played in 200ms

	startTime = streamTime + 0.2;

	opacket.startTime = startTime;
	opacket.size = 2;
	opacket.stride = 2;
	opacket.fp32 = iqbuffer;
	opacket.num = 16384;

	// Prepare input packet
	AARTSAAPI_Packet	ipacket = { sizeof(AARTSAAPI_Packet) };

	// Stream 1000 packets

	int NumPackets = 1000;

	int i = 0;
	while (i < NumPackets)
	{
		AARTSAAPI_Result	res;

		// Packet end time
		opacket.endTime = opacket.startTime + opacket.num / opacket.stepFrequency;

		AARTSAAPI_GetMasterStreamTime(&d, streamTime);

		// Check if it is time to send a new output packet
		if (streamTime + 0.05 >= opacket.startTime)
		{
			if (i == 0)
				opacket.flags = AARTSAAPI_PACKET_SEGMENT_START | AARTSAAPI_PACKET_STREAM_START;
			else if (i + 1 == NumPackets)
				opacket.flags = AARTSAAPI_PACKET_SEGMENT_END | AARTSAAPI_PACKET_STREAM_END;
			else
				opacket.flags = 0;

			// Send the packet

			AARTSAAPI_SendPacket(&d, 0, &opacket);

			// Advance packet time

			opacket.startTime = opacket.endTime;
			i++;
		}
		else if ((res = AARTSAAPI_GetPacket(&d, 0, 0, &ipacket)) != AARTSAAPI_EMPTY)
		{

			// If we actually got a packet

			if (res == AARTSAAPI_OK)
			{

				// Do something with it
				std::wcout 
					<< "In " 
					<< ipacket.startTime - startTime << ".." << ipacket.endTime - startTime << " " 
					<< ipacket.num << " " << ipacket.stepFrequency << " : " 
					<< log10(ipacket.fp32[0] * ipacket.fp32[0] + ipacket.fp32[1] * ipacket.fp32[1]) * 10 << ", "
					<< log10(ipacket.fp32[ipacket.num] * ipacket.fp32[ipacket.num] + ipacket.fp32[ipacket.num + 1] * ipacket.fp32[ipacket.num + 1]) * 10
					<< std::endl;
			}

			// Consume the packet from the receiver queue

			AARTSAAPI_ConsumePackets(&d, 0, 1);
		}
		else
		{
			// Wait five milliseconds for more, shorter duration if sample
			// rate is higher

			std::this_thread::sleep_for( std::chrono::milliseconds(5));
		}
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
						AARTSAAPI_Config	config, root;

						if (AARTSAAPI_ConfigRoot(&d, &root) == AARTSAAPI_OK)
						{
							// Select the first receiver channel

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/receiverchannel") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetString(&d, &config, L"Rx1");

							// Select the center frequency of the tuner

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/centerfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 2440.0e6);

							// Select the frequency range of the tuner

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/spanfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 50.0e6);

							// Select the frequency range of the receiver demodulator to pick up a
							// frequency range from the input stream

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/demodcenterfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 2430.5e6);

							// Select the frequency span of the receiver demodulator

							if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/demodspanfreq") == AARTSAAPI_OK)
								AARTSAAPI_ConfigSetFloat(&d, &config, 1.0e6);

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
