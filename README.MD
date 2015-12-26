DIY-Thermocam Firmware Development 

www.diy-thermocam.net

Download and install the following programs:

1. Arduino 1.6.6: https://www.arduino.cc/en/Main/OldSoftwareReleases#previous
2. Teensyduino 1.26: https://www.pjrc.com/teensy/td_download.html
3. Visual Studio 2015 Community Edition: https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx
4. Visual Micro: http://www.visualmicro.com/page/Arduino-Visual-Studio-Downloads.aspx

Now Start Visual Studio.

In the "Configure IDE Locations" window under Tools -> VisualMicro, point to the Arduino IDE folder and the Libraries folder from this project.

In the Visual Micro menu select the following settings:

IDE: Arduino 1.6
Board: Teensy 3.1/3.2
USB Type: Serial
CPU Speed 144MHz 

Finally, select the right Comport from the list. 
If there is no Comport, upload a blank sketch with the USB Type "Serial" to the Arduino first over the Arduino IDE.

Import the project with Open -> Arduino Project. 

Add all the Libraries from the "libraries" folder to the SDK by choosing one after another under Project -> Add Library -> built-in.
You can view them by selecting Project -> Show Arduino Core and Library Sources.

Compile the project with F7. The last step is to include the Mass Storage into the Hex file.

Copy the output hex "DIY-Thermocam.hex" from Visual studio into the folder "MSD" and start the file "Unify.bat". A new file called "Firmware.hex" will be created.

Upload this file to your DIY-Thermocam with the Teensy.exe uploader from the "MSD" folder.