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

Happy Building!