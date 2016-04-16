/*
*
* DIY-Thermocam Firmware
*
* 2014-2016 by Max Ritter
*
* http://www.diy-thermocam.net
* https://github.com/maxritter/DIY-Thermocam
*
*/

/* Current firmware version */
#define Version "Firmware 1.17 from 16.04.2016"

/* Libraries */
#include <ADC.h>
#include <i2c_t3.h>
#include <SdFat.h>
#include <tjpgd.h>
#include <SPI.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <Bounce.h>
#include <UTFT_Buttons.h>
#include <UTFT.h>
#include <Touchscreen.h>
#include <Camera.h>

/* General Includes */
#include "General/ColorSchemes.h"
#include "General/GlobalItems.h"
#include "General/MethodDefines.h"

/* Modules */
#include "Hardware/Hardware.h"
#include "GUI/GUI.h"
#include "Thermal/Thermal.h"


/* Main Entry point */
void setup()
{
	//Init Hardware
	initHardware();
	//Check for hardware issues
	if (diagnostic != 0xFF) {
		//Show the diagnostics over serial
		printDiagnostic();
		//Show it on the screen
		showDiagnostic();
		//Wait for touch press
		while (!touch.touched());	
		//Wait for touch release
		while (touch.touched());
	}
	//Read EEPROM settings
	readEEPROM();
	//Go to the live Mode
	liveMode();
}

/* Loop forever */
void loop()
{
	//Main Menu Handler
	mainMenuHandler();
}
