# BluetoothTerminal
This is a simple Bluetooth Terminal application designed to be used with the Adafruit BLE UART friend. I developed this because I could not find a way to link the UART friend to a COM port on my Win10 machine. This program communicates directly with the UART GATT service instead. This project was created in Qt. The compiler I used was MSVC2015 64 bit. This should work across platforms but I have not tested it.

# Installation Instructions
1. Navigate to the installer under the "Installer" directory.
2. Run the installer ("BluetoothTerminal_Win64.exe")
3. Install the software as default, or change the directory/start menu configuration.
4. The application is now ready to be run.

# Build Instructions
1. Install Qt Creator (I used version 5.11.2).
2. Install the MSVC 2015 64 bit compiler, or any compiler of your choice (I cannot guarantee that other compilers will work).
3. Clone the repository to your local computer
4. Open Qt Creator.
5. Navigate to the .pro file and open it
6. Click the "Build & Run" green arrow on the bottom left. After a delay the application should start.
