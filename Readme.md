# Building the Aaronia RTSA-Suite PRO SDK Samples

This document provides instructions on how to build the Aaronia RTSA-Suite PRO SDK sample projects using CMake on both Windows and Linux operating systems.

For questions & issues please visit: https://v6-forum.aaronia.de/

## 1. Prerequisites

Before you begin, ensure you have the following software installed on your system:

**General:**
* **Aaronia RTSA-Suite PRO:** The samples rely on the SDK files installed with the Aaronia RTSA-Suite PRO. Make sure it's installed in the default location, or be prepared to adjust the paths in the main `CMakeLists.txt` file.
    * Default Windows Path: `C:\Program Files\Aaronia AG\Aaronia RTSA-Suite PRO`
    * Default Linux Path: `/opt/aaronia-rtsa-suite/Aaronia-RTSA-Suite-PRO`
* **CMake:** Version 3.16 or higher. You can download it from [https://cmake.org/download/](https://cmake.org/download/).

**Windows:**
* **C++ Compiler:** A C++ compiler is required. The easiest way to get this is by installing Visual Studio.
    * **Visual Studio:** Install Visual Studio (e.g., Visual Studio 2019 or later) with the "Desktop development with C++" workload. This will include MSVC (Microsoft Visual C++ compiler), CMake tools, and other necessary build utilities.
    * Alternatively, you can use VSCode with the C++ Extension Pack and separately install the MSVC build tools.

**Linux:**
* **C++ Compiler:** A C++ compiler like GCC or Clang.
    * Typically installed via your distribution's package manager. For example, on Debian/Ubuntu:
      ```bash
      sudo apt update
      sudo apt install build-essential g++ cmake
      ```
* **Make:** The `make` utility is commonly used with CMake on Linux. (Usually included with `build-essential`).

## 2. Setup: Copying the Samples Folder

The SDK sample projects are typically installed in a system directory that might not be writable by a standard user or could be overwritten during software updates. It's highly recommended to copy the `Samples` folder to a user-writable location before attempting to build.

**Linux:**
1.  The default location of the samples is:
    `/opt/aaronia-rtsa-suite/Aaronia-RTSA-Suite-PRO/sdk/Samples`
2.  Copy this entire `Samples` folder to a writable location in your home directory, for example:
    `~/dev/rtsa_sdk_samples`
    You can do this using the terminal:
    ```bash
    cp -r /opt/aaronia-rtsa-suite/Aaronia-RTSA-Suite-PRO/sdk/Samples ~/dev/rtsa_sdk_samples
    cd ~/dev/rtsa_sdk_samples
    ```

**Windows:**
1.  The default location of the samples is:
    `C:\Program Files\Aaronia AG\Aaronia RTSA-Suite PRO\sdk\Samples`
2.  Copy this entire `Samples` folder to a writable location, for example:
    `C:\temp\RTSA_SDK_SAMPLES` or `Documents\Development\RTSA_SDK_SAMPLES`


**Important:** All subsequent build commands should be run from within your *copied* `Samples` directory.

## 3. Building the Samples

Ensure you are in the root of your *copied* `Samples` directory for the following steps.

**Linux (Command Line):**
1.  Open a terminal and navigate to your copied `Samples` directory:
    ```bash
    cd /path/to/your/copied/Samples
    ```
2.  Create a build directory and navigate into it:
    ```bash
    mkdir build
    cd build
    ```
3.  Run CMake to configure the project. This will generate the Makefiles:
    ```bash
    cmake ..
    ```
    *Note: `..` refers to the parent directory (your copied `Samples` directory where the main `CMakeLists.txt` is located).*
4.  Compile the projects using make:
    ```bash
    make
    ```
    To speed up compilation on multi-core processors, you can use the `-j` flag (e.g., `make -j4` to use 4 cores).
5.  The compiled executables will be located in subdirectories within the `build` directory (e.g., `build/IQReceiver/IQReceiver`).

**Windows:**

There are two primary recommended ways to build on Windows:

**Option A: Using Visual Studio IDE**
1.  Ensure Visual Studio with the "Desktop development with C++" workload is installed.
2.  Open Visual Studio.
3.  Select "Open a local folder" or "File" > "Open" > "Folder...".
4.  Navigate to and select your *copied* `Samples` directory (e.g., `C:\temp\RTSA_SDK_SAMPLES`).
5.  Visual Studio should automatically detect the `CMakeLists.txt` file and start configuring the project. You can monitor the progress in the "Output" window (Show output from: CMake).
6.  Once CMake configuration is complete, you can select the build type (e.g., Debug, Release) and the target executable from the toolbar.
7.  Build the project by selecting "Build" > "Build All" from the menu, or by right-clicking the `CMakeLists.txt` in the Solution Explorer and choosing "Build".
8.  The compiled executables will typically be found in a subdirectory like `out/build/<config-type>/<SampleName>/<SampleName.exe>` (e.g., `out/build/x64-Debug/IQReceiver/IQReceiver.exe`).

**Option B: Using Visual Studio Code (VSCode) with CMake Tools Extension**
1.  Ensure you have VSCode, the Microsoft C/C++ Extension Pack, and the CMake Tools extension installed.
2.  Ensure you have a C++ compiler accessible (e.g., MSVC build tools installed with Visual Studio configured correctly).
3.  Open VSCode.
4.  Select "File" > "Open Folder..." and navigate to your *copied* `Samples` directory.
5.  VSCode and the CMake Tools extension should prompt you to configure the project or ask to select a kit (compiler).
    * If prompted for a kit, choose your preferred compiler (e.g., Visual Studio Community 20XX Release - amd64).
    * The CMake Tools extension will then configure the project. You can see output in the "Output" panel (select "CMake" from the dropdown).
6.  Once configured, you can build:
    * Use the "Build" button (often a gear or hammer icon) in the status bar provided by CMake Tools.
    * Or open the command palette (Ctrl+Shift+P) and type "CMake: Build".
7.  The compiled executables will be located in a `build` subdirectory (e.g., `build/IQReceiver/IQReceiver.exe`). The exact path might vary based on your CMake Tools settings.

## 4. Important Notes

* **Aaronia RTSA Installation Path:** The main `CMakeLists.txt` file attempts to locate your Aaronia RTSA-Suite PRO installation.
    ```cmake
    # WINDOWS:
    set(AARONIA_RTSA_INSTALL_DIRECTORY "C:\\Program Files\\Aaronia AG\\Aaronia RTSA-Suite PRO")
    # LINUX:
    set(AARONIA_RTSA_INSTALL_DIRECTORY "/opt/aaronia-rtsa-suite/Aaronia-RTSA-Suite-PRO")
    ```
    If your installation path differs, you **MUST** update these lines in the `CMakeLists.txt` file (located in the root of your copied `Samples` directory) *before* running `cmake ..` or opening the folder in Visual Studio / VSCode.

* **SDK Path:** The CMake script automatically determines the SDK directory based on the `AARONIA_RTSA_INSTALL_DIRECTORY`.

* **Troubleshooting:**
    * If CMake cannot find the RTSA API (`find_package(RTSAAPI REQUIRED)` fails), ensure the `AARONIA_RTSA_INSTALL_DIRECTORY` is set correctly and that the SDK was properly installed with the RTSA-Suite PRO.
    * On Linux, ensure all development tools (`build-essential`, `g++`, `cmake`) are installed.
    * On Windows, ensure the C++ workload in Visual Studio is installed, or that your chosen compiler is correctly configured in the system PATH or for CMake Tools in VSCode.

* **SDK Wrapping**
We've seen various attempts to wrap the API and therefore provide a working sample (Work in progress) in the Wrapper directory. This is not included in the main CMakeLists.txt file intentionally. To build it copy the Wrapper directory to a writable location, and build with the same procedure as described for the Samples root directory.




# Usage notes

## Device enumaration

For details see `EnumDevices/EnumDevices.cpp`

V6 Device: "spectranv6"

```
if ((res = AARTSAAPI_EnumDevice(&h, L"spectranv6", 0, &dinfo)) == AARTSAAPI_OK)
{
...
}
```


V6 ECO Device: "spectranv6eco"

```
if ((res = AARTSAAPI_EnumDevice(&h, L"spectranv6eco", 0, &dinfo)) == AARTSAAPI_OK)
{
...
}
```


## Modes Overview

The **SPECTRAN V6** and **V6 ECO** devices can operate in different modes, primarily **RAW** mode and **IQ mode**.

- **RAW Mode**: This mode provides direct access to the raw data from the device. The data packets arrive at the base clock speed, which can be configured for V6 devices (or is fixed for V6 ECO). You can then apply decimation to reduce the sample rate. This mode gives you fine-grained control over the data stream.
- **IQ Mode**: This mode is designed for easier handling, especially when using the V6 or V6 ECO **as a transmitter or transceiver in Software Defined Radio (SDR) applications**. The RTSA SDK internally uses modulators and demodulators. Instead of setting a decimation factor directly, you configure the desired spanfreq, and the SDK handles the necessary internal adjustments to achieve an appropriate sample rate. This mode is optimal for arbitrary sample rates.


### RAW

Coding samples: `RawIQ`, `RawIQ2RX`, `RawIQ2RxInterleave`

Open the device in RAW mode:

V6 Device:
```
if ((res = AARTSAAPI_OpenDevice(&h, &d, L"spectranv6/raw", dinfo.serialNumber)) == AARTSAAPI_OK)
{
...
}
```

V6 ECO Device:
```
if ((res = AARTSAAPI_OpenDevice(&h, &d, L"spectranv6eco/raw", dinfo.serialNumber)) == AARTSAAPI_OK)
{
...
}
```

The packets come in at the desired base clock speed, set via `device/receiverclock`.

For V6 DeviceTypes you can set the `receiverclock` using:

```
if (AARTSAAPI_ConfigFind(&d, &root, &config, L"device/receiverclock") == AARTSAAPI_OK)
    AARTSAAPI_ConfigSetString(&d, &config, L"92MHz");
```

| ConfigItem    | ActualRate |
| -------- | ------- |
|`46MHz` |46.08 MHz|
|61MHz|61.44 MHz|
|76MHz|76.80 MHz|
|92MHz|92.16 MHz|
|122MHz|122.88 MHz|
|245MHz|245.76 MHz|


V6 ECO devices have no `device/receiverclock` ConfigItem and have a fixed rate at `61.44 MHz`.

Decimation uses half-band filters to reduce the sample rate:

```
if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/decimation") == AARTSAAPI_OK)
    AARTSAAPI_ConfigSetString(&d, &config, L"1 / 64");
```
Valid values for decimation are:
- full
- 1 / 2
- 1 / 4
- 1 / 8
- 1 / 16
- 1 / 32
- 1 / 64
- 1 / 128
- 1 / 256
- 1 / 512

When using decimation, the resulting sample rate = ActualRate / Decimation, like:

```
decimation 1/4
receiverclock 92.16 MHz
sample rate = 23.04 MHz (92.16 / 4)
```

### IQ modes

Compared to RawMode, the RTSA SDK internally uses modulators/demodulators to make handling the device easier.
It will especially be easier when using the V6 or V6 ECO as a transmitter or transceiver device in SDR applications.

Coding samples: IQReceiver, IQReceiverEco, IQTransmitter, IQTransmitterEco, IQTranceiver, IQTranceiverEco

This mode offers optimal handling of the V6 / V6 ECO when using arbitrary sample rates.

There is an important concept to understand here:

Instead of setting a decimation, you set the main/spanfreq like:

```
if (AARTSAAPI_ConfigFind(&d, &root, &config, L"main/spanfreq") == AARTSAAPI_OK)
AARTSAAPI_ConfigSetFloat(&d, &config, 10.0e6);
```

This setting comes from the nature of the V6 being a **spectrum analyzer, not an SDR by nature.**

The resulting sample rate of the packet object is `main/spanfreq * 1.5`. This is due to how internal decimation and filters work.

Some examples:

`Desired sample rate = 15 MHz`, you have to set the span to `10 MHz`.

When debugging, `packet.stepFrequency` value tells you the actual sample rate if unsure.

**Keep in mind, that spanfreq can not be higher than receiverclock / 1.5.**


## General hits

### Rescan & Enumerate devices

When first connecting a device, it might be nessesary to call `AARTSAAPI_RescanDevices` repeatedly. Implement a robust loop, that rescans and re-enumerates devices based on your needs.

### Device Health Status & Configration

During operation `AARTSAAPI_ConfigRoot` and `AARTSAAPI_ConfigHealth` can be queried to get the current device health state and configuration.

Also during development. When unsure, about what config items are available, you can always use the `ConfigTree` example.
Just adjust the device type in `AARTSAAPI_EnumDevice` argument and `AARTSAAPI_OpenDevice` to query the full tree.

Valid `AARTSAAPI_OpenDevice` `type` arguments: 

V6:
- `spectranv6/raw`
- `spectranv6/iqreceiver`
- `spectranv6/iqreceiver`
- `spectranv6/iqtransceiver`
- `spectranv6/iqtransmitter`
- `spectranv6/spertsa`

V6 ECO:
- `spectranv6eco/raw`
- `spectranv6eco/iqreceiver`
- `spectranv6eco/iqreceiver`
- `spectranv6eco/iqtransceiver`
- `spectranv6eco/iqtransmitter`
- `spectranv6eco/spertsa`


adjusted `ConfigTree.cpp` 
```
...
	if ((res = AARTSAAPI_EnumDevice(&h, L"spectranv6eco", 0, &dinfo)) == AARTSAAPI_OK)
				{
					AARTSAAPI_Device	d;

					// Try to open the first V6 in the system in raw mode

					if ((res = AARTSAAPI_OpenDevice(&h, &d, L"spectranv6eco/iqreceiver", dinfo.serialNumber)) == AARTSAAPI_OK)
					{
						AARTSAAPI_Config	root;
...
```

An example output for an idle SPECTRAN V6 ECO opened as spectranv6eco/iqreceiver: 
```
-> % /home/f102/wrk/RTSA-API-Samples/build/ConfigTree/ConfigTreeExample
CONFIG:
main(Main, , ),  : ""
. centerfreq(Center Frequency, Frequency, ),  : "2.44e+09"
. reflevel(Reference Level, dBm, ),  : "0"
. spanfreq(Span Frequency, Frequency, ),  : "4e+07"
device(Board Config, , ),  : ""
. loharmonics(Frequency Range, , auto;6ghz;18ghz;30ghz;42ghz;54ghz;66ghz;78ghz;90ghz;102ghz;114ghz;126ghz;138ghz;150ghz),  : "auto"
. usbcompression(USB Compression, , auto;compressed;raw),  : "auto"
. generator(Signal Generator, , ),  : ""
. . type(Generator Type, , Relative Tone;Absolute Tone;Step;Random Step;Sweep;Sweep Alternate;Full Sweep;Center Sweep;Polytone;Relative Ditone;Absolute Ditone;Noise;Digital Noise;Off),  : "Relative Tone"
. . startfreq(Start Frequency, Frequency, ),  : "2.4e+09"
. . stopfreq(Stop Frequency, Frequency, ),  : "2.42e+09"
. . stepfreq(Step Frequency, Frequency, ),  : "1e+06"
. . offsetfreq(Offset Frequency, Frequency, ),  : "1e+06"
. . duration(Duration, Time, ),  : "10"
. . powerramp(Power Ramp, dB, ),  : "0"
. . softstart(Soft Start, Percentage, ),  : "0"
. sclksource(Stream Clock Source, , Consumer;Oscillator;GPS;PPS;10MHz;Oscillator Provider;GPS Provider;PPS Provider),  : "Consumer"
. triggeredge(Trigger Edge, , Off;High;Low;Rising;Falling;Changing),  : "Off"
. triggerflag(Trigger Flag, , C0;C1;C2;C3),  : "C0"
. centeroffsetenable(Center Freq. Offset Enable, , ),  : "false"
. centeroffset(Center Freq. Offset, Frequency, ),  : "1.3e+06"
. gpsmode(GPS Mode, , Disabled;Location;Time;Location and Time),  : "Disabled"
. gpsrate(GPS Update Rate, Time, ),  : "0.5"
. sweepphases(Sweep Phases, , ),  : "1"
. transmittermode(Transmitter Mode, , Off;Test;Stream;Signal Generator;Pattern Generator),  : "Off"
. transmitterclockvar(Tx Clock Tolerance, Time, ),  : "0.0002"
. transmitterautomute(Tx Auto Mute, , ),  : "false"
. pwrmeterenabled(Power Meter Enabled, , ),  : "true"
. serial(Serial Number, , ),  : ""
calibration(Calibration, , ),  : ""
. bypassfilter(Bypass Filter, , ),  : "false"
. rffilter(RX Filter, , Calibration;Bypass;Auto;Upconverter;240-545;340-650;440-765;610-1005;850-1370;1162-2060;1850-3010;2800-4610;4400-6025;5925-7500),  : "Auto"
. preamp(RF Amplifier, , auto;off;amp1;amp2),  : "auto"
. calibrationmode(Mode, , Off;RX Attenuator;TX Attenuator;Tx No Amplifier;Tx Amplifier;Rx Thermal;Tx Thermal;Rx RTBW;Tx RTBW;Rx Filter;Rx Amplifier;Tx LO Leakage;Clock;Powermeter;Raw;Free),  : "Off"
. txioffset(Tx I-Offset, Number, ),  : "0"
. txqoffset(Tx Q-Offset, Number, ),  : "0"
. txexcent(Tx IQ-Excentricity, Percentage, ),  : "0"
. txphaseskew(Tx IQ-Phase Skew, Degree, ),  : "0"
. calibrationreload(Reload, , ),  : "0"
. clockscale(Clock offset at 100MHz, Frequency, ),  : "0"
. clockbygpsupdate(Calibrate Clock by GPS, , Never;Once;Reset;On Startup;Slow;Fast;Realtime),  : "Never"
. SpectranV6EcoDCOffset1(DC Offset Correction, , ),  : ""
. . frequency(DC Corr Frequency, Frequency, ),  : "5000"
. . power(DC Corr Attn, Number, ),  : "120"
. . maxpower(DC Corr Limit, Number, ),  : "20"
. . bypass(DC Corr Bypass, , ),  : "false"
. . iir(DC Corr LP IIR, Percentage, ),  : "1"
. . iirdecar(DC Corr LP IIR Decay, Percentage, ),  : "0.125"
. . processflagdc(Only DC process packets, , ),  : "true"
. SpectranV6EcoDCFastOffset1(DC Offset Correction (Fast), , ),  : ""
. . frequency(DC Corr Frequency, Frequency, ),  : "400000"
. . power(DC Corr Attn, Number, ),  : "10"
. . maxpower(DC Corr Limit, Number, ),  : "-70"
. . bypass(DC Corr Bypass, , ),  : "false"
. . iir(DC Corr LP IIR, Percentage, ),  : "0.6"
. . iirdecar(DC Corr LP IIR Decay, Percentage, ),  : "0.125"
. . processflagdc(Only DC process packets, , ),  : "true"
. SpectranV6EcoNormalizer1(IQ Normalization, , ),  : ""
. . frequency(Corr LP FIR, Frequency, ),  : "5000"
. . iir(Corr LP IIR, Percentage, ),  : "0.001"
. . iirdecar(Corr LP IIR Decay, Percentage, ),  : "0.125"
. . overscale(Overscale Exc, Percentage, ),  : "0"
. . correctdc(Correct DC, , ),  : "false"
. . correctexc(Correct Excentricity, , ),  : "true"
. . correctphase(Correct Phase, , ),  : "true"
. . processflagqec(Only QEC process packets, , ),  : "true"

STATUS:
rx1iqsamplessecond(Rx1 IQ Samples/s, Number, ),  : "0"
rx2iqsamplessecond(Rx2 IQ Samples/s, Number, ),  : "0"
tx1iqsamplessecond(Tx IQ Samples/s, Number, ),  : "0"
rx1InputPower(Rx1 input power dB, Number, ),  : "-100"
rx2InputPower(Rx2 input power dB, Number, ),  : "-100"
errors(Errors, , ),  : "0"
reconnects(Reconnects, , ),  : "0"
busresets(Bus Resets, , ),  : "0"
restarts(Restarts, , ),  : "0"
retries(Command Retries, , ),  : "0"
errorssecond(Errors/s, Number, ),  : "0"
usboverflowssecond(USB Overflows/s, Number, ),  : "0"
frontendtemp(Frontend Temperature, Number, ),  : "0"
fpgatemp(FPGA Temperature, Number, ),  : "0"
mainusbbytessecond(Main USB Bytes/s, Number, ),  : "0"
gpsposvalid(GPS Position Valid, , ),  : "false"
gpstimevalid(GPS Time Valid, , ),  : "false"
gpssats(GPS Satellites, , ),  : "0"
gpslatitude(GPS Latitude, GeoCoord, ),  : "0"
gpslongitude(GPS Longitude, GeoCoord, ),  : "0"
gpselevation(GPS Elevation, Meter, ),  : "0"
gpstime(GPS Time, DateTime, ),  : "0"
gpstimeoffset(GPS Time Offset, Time, ),  : "0"
```


`spectranv6` opened as `spectranv6/raw`

```
-> % /home/f102/wrk/RTSA-API-Samples/build/ConfigTree/ConfigTreeExample
CONFIG:
main(Main, , ),  : ""
. centerfreq(Center Frequency, Frequency, ),  : "2.44e+09"
. decimation(Span, , Full;1 / 2;1 / 4;1 / 8;1 / 16;1 / 32;1 / 64;1 / 128;1 / 256;1 / 512),  : "Full"
. reflevel(Reference Level, dBm, ),  : "0"
. transgain(Transmitter Gain, dB, ),  : "-20"
device(Board Config, , ),  : ""
. usbcompression(USB Compression, , auto;compressed;raw),  : "auto"
. gaincontrol(Gain Control, , manual;peak;power),  : "manual"
. frequencyrange(Frequency Range, , auto;6ghz;18ghz),  : "6ghz"
. outputformat(Output Format, , iq;spectra;both;auto),  : "auto"
. lowpower(Low Power Mode, , ),  : "false"
. fft0(Spectra 1, , ),  : ""
. . fftmergemode(FFT Merge Mode, , avg;sum;min;max),  : "avg"
. . fftaggregate(FFT Merge Length, , ),  : "1"
. . fftsizemode(FFT Size Mode, , FFT;Bins;Step Frequency;RBW),  : "FFT"
. . fftsize(FFT Size, , ),  : "2048"
. . fftbinsize(FFT Bins, , ),  : "2048"
. . fftstepfreq(FFT Step Frequency, Frequency, ),  : "100000"
. . fftrbwfreq(FFT RBW Frequency, Frequency, ),  : "100000"
. . fftwindow(FFT Window, , Hamming;Hann;Uniform;Blackman;Blackman Harris;Blackman Harris 7;Flat Top;Lanczos;Gaussion 0.5;Gaussion 0.4;Gaussian 0.3;Gaussion 0.2;Gaussian 0.1;Kaiser 6;Kaiser 12;Kaiser 18;Kaiser 36;Kaiser 72;Tukey 0.1;Tukey 0.3;Tukey 0.5;Tukey 0.7;Tukey 0.9),  : "Hamming"
. fft1(Spectra 2, , ),  : ""
. . fftmergemode(FFT Merge Mode, , avg;sum;min;max),  : "avg"
. . fftaggregate(FFT Merge Length, , ),  : "1"
. . fftsizemode(FFT Size Mode, , FFT;Bins;Step Frequency;RBW),  : "FFT"
. . fftsize(FFT Size, , ),  : "2048"
. . fftbinsize(FFT Bins, , ),  : "2048"
. . fftstepfreq(FFT Step Frequency, Frequency, ),  : "100000"
. . fftrbwfreq(FFT RBW Frequency, Frequency, ),  : "100000"
. . fftwindow(FFT Window, , Hamming;Hann;Uniform;Blackman;Blackman Harris;Blackman Harris 7;Flat Top;Lanczos;Gaussion 0.5;Gaussion 0.4;Gaussian 0.3;Gaussion 0.2;Gaussian 0.1;Kaiser 6;Kaiser 12;Kaiser 18;Kaiser 36;Kaiser 72;Tukey 0.1;Tukey 0.3;Tukey 0.5;Tukey 0.7;Tukey 0.9),  : "Hamming"
. receiverclock(Receiver Clock, , 92MHz;122MHz;184MHz;245MHz;492MHz;77MHz;61MHz;46MHz),  : "92MHz"
. receiverchannel(Receiver Channels, , Rx1;Rx2;Rx1+Rx2;Rx1/Rx2;Rx12;Rx Off;auto),  : "auto"
. receiverchannelsel(Receiver Select, , Rx1;Rx2;Rx2->1;Rx1->2),  : "Rx1"
. sweepphases(Sweep Phases, , ),  : "1"
. transmittermode(Transmitter Mode, , Off;Test;Stream;Reactive;Signal Generator;Pattern Generator),  : "Off"
. transmitterclockvar(Tx Clock Tolerance, Time, ),  : "0.0002"
. fasttune(Fast Tune, , ),  : "false"
. generator(Signal Generator, , ),  : ""
. . type(Generator Type, , Relative Tone;Absolute Tone;Step;Random Step;Sweep;Sweep Alternate;Full Sweep;Center Sweep;Polytone;Relative Ditone;Absolute Ditone;Noise;Digital Noise;Off),  : "Relative Tone"
. . startfreq(Start Frequency, Frequency, ),  : "2.4e+09"
. . stopfreq(Stop Frequency, Frequency, ),  : "2.42e+09"
. . stepfreq(Step Frequency, Frequency, ),  : "1e+06"
. . offsetfreq(Offset Frequency, Frequency, ),  : "1e+06"
. . duration(Duration, Time, ),  : "10"
. . powerramp(Power Ramp, dB, ),  : "0"
. . softstart(Soft Start, Percentage, ),  : "0"
. sclksource(Stream Clock Source, , Consumer;Oscillator;GPS;PPS;10MHz;Oscillator Provider;GPS Provider;PPS Provider),  : "Consumer"
. triggeredge(Trigger Edge, , Off;High;Low;Rising;Falling;Changing),  : "Off"
. triggerflag(Trigger Flag, , C0;C1;C2;C3),  : "C0"
. centeroffsetenable(Center Freq. Offset Enable, , ),  : "false"
. centeroffset(Center Freq. Offset, Frequency, ),  : "1.3e+06"
. gpsmode(GPS Mode, , Disabled;Location;Time;Location and Time),  : "Disabled"
. gpsrate(GPS Update Rate, Time, ),  : "0.5"
. tempfancontrol(Temperature Fan Control, , ),  : "false"
. disablefpgaversion(Disable FPGA Version Check, , ),  : "false"
. dspbufmode(DSP Buffer Mode, , Auto;Min Latency;Max Throughput;Max Resilience),  : "Auto"
. serial(Serial Number, , ),  : ""
calibration(Calibration, , ),  : ""
. bypassfilter(Bypass Filter, , ),  : "false"
. rffilter(RX Filter, , Calibration;Bypass;Auto;Auto Extended;75-145 (50);90-160 (50);110-195 (50);135-205 (50);155-270 (50);155-270 (100);155-280 (100);180-350 (100);230-460 (100);240-545;340-650;440-815;610-1055;850-1370;1162-2060;1850-3010;2800-4610;4400-6100),  : "Auto"
. preamp(RF Amplifier, , Disabled;Auto;None;Amp;Preamp;Both),  : "Disabled"
. rftxfilter(TX Filter, , Calibration;Bypass;Auto;Auto Extended;75-145 (50);90-160 (50);110-195 (50);135-205 (50);155-270 (50);155-270 (100);155-280 (100);180-350 (100);230-460 (100);240-545;340-650;440-815;610-1055;850-1370;1162-2060;1850-3010;2800-4610;4400-6100),  : "Auto"
. calibrationmode(Mode, , Off;RX Attenuator;TX Attenuator;Tx No Amplifier;Tx Amplifier;Rx Thermal;Tx Thermal;Rx RTBW;Tx RTBW;Rx Filter;Rx Amplifier;Tx LO Leakage;Clock;Raw;Free),  : "Off"
. trackingcal(Tracking Calibration, , ),  : "true"
. ddccorrection(HW DC/QEC Correction, , ),  : "true"
. txioffset(Tx I-Offset, Number, ),  : "0"
. txqoffset(Tx Q-Offset, Number, ),  : "0"
. txexcent(Tx IQ-Excentricity, Percentage, ),  : "0"
. txphaseskew(Tx IQ-Phase Skew, Degree, ),  : "0"
. clockscale(Clock offset at 100MHz, Frequency, ),  : "0"
. clockbygpsupdate(Calibrate Clock by GPS, , Never;Once;Reset;On Startup;Slow;Fast;Realtime),  : "Never"
. calibrationreload(Reload, , ),  : "0"

STATUS:
rx1iqsamplessecond(Rx1 IQ Samples/s, Number, ),  : "0"
rx2iqsamplessecond(Rx2 IQ Samples/s, Number, ),  : "0"
tx1iqsamplessecond(Tx IQ Samples/s, Number, ),  : "0"
rx1spectrasecond(Rx1 Spectra/s, Number, ),  : "0"
rx2spectrasecond(Rx2 Spectra/s, Number, ),  : "0"
rx1InputPower(Rx1 input power dB, Number, ),  : "50"
rx2InputPower(Rx2 input power dB, Number, ),  : "50"
errors(Errors, , ),  : "0"
reconnects(Reconnects, , ),  : "0"
busresets(Bus Resets, , ),  : "0"
restarts(Restarts, , ),  : "0"
retries(Command Retries, , ),  : "0"
errorssecond(Errors/s, Number, ),  : "0"
usboverflowssecond(USB Overflows/s, Number, ),  : "0"
frontendtemp(Frontend Temperature, Number, ),  : "0"
fpgatemp(FPGA Temperature, Number, ),  : "0"
mainusbbytessecond(Main USB Bytes/s, Number, ),  : "0"
boostusbbytessecond(Boost USB Bytes/s, Number, ),  : "0"
gpsposvalid(GPS Position Valid, , ),  : "false"
gpstimevalid(GPS Time Valid, , ),  : "false"
gpssats(GPS Satellites, , ),  : "0"
gpslatitude(GPS Latitude, GeoCoord, ),  : "0"
gpslongitude(GPS Longitude, GeoCoord, ),  : "0"
gpselevation(GPS Elevation, Meter, ),  : "0"
gpstime(GPS Time, DateTime, ),  : "0"
gpstimeoffset(GPS Time Offset, Time, ),  : "0"
```
