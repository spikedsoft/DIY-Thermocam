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
#define Version "Firmware 1.19 from 10.06.2016"
#define fwVersion 119

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
#include "General/GlobalVariables.h"
#include "General/GlobalMethods.h"

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
	checkDiagnostic();
	//Do the first start setup
	if (EEPROM.read(eeprom_firstStart) != eeprom_setValue)
		firstStart();
	//Check FW upgrade
	checkFWUpgrade();
	//Read EEPROM settings
	readEEPROM();
	//Show the live mode helper
	if (EEPROM.read(eeprom_liveHelper) != eeprom_setValue)
		liveModeHelper();
	//Go to the live Mode
	liveMode();
}

/* Loop forever */
void loop()
{
	//Main Menu Handler
	mainMenuHandler();
}
