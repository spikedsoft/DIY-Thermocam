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
#define Version "FW Verson 1.03"

/* Libraries */
#include <ADC.h>
#include <i2c_t3.h>
#include <SdFat.h>
#include <SPI.h>
#include <Time.h>
#include <EEPROM.h>
#include <Bounce.h>
#include <XPT2046_Touchscreen.h>
#include <UTFT_Buttons.h>
#include <UTFT.h>
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
	//Read EEPROM settings
	if (checkEEPROM()) {
		//Return to connection menu when coming from Mass storage
		connectionMenu();
		connectionMenuHandler();
	}
	else
		//Go to the live Mode
		liveMode();
}

/* Loop forever */
void loop()
{
	//Main Menu Handler
	mainMenuHandler();
}
