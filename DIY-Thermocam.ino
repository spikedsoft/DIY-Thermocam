/*
* DIY-Thermocam Firmware
*
* 2015 by Max Ritter
*
* www.diy-thermocam.net
*
*/

/* Current firmware version */
#define Version "FW Verson 1.01"

/* Libraries */
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

/* Modules */
#include "Hardware/Hardware.h"
#include "GUI/GUI.h"
#include "Thermal/Thermal.h"

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

void loop()
{
	mainMenuHandler();
}
