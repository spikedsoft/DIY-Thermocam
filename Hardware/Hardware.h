/*
* Main hardware functions
*/

/* Includes */
#include "Battery.h"
#include "Cam.h"
#include "MLX90614.h"
#include "Lepton.h"
#include "SD.h"
#include "Connection.h"
#include "MassStorage.h"

/* Methods */

/* Get time from the RTC */
time_t getTeensy3Time()
{
	return Teensy3Clock.get();
}

/* Switches the laser on or off*/
void toggleLaser() {
	if (laserEnabled) {
		digitalWrite(pin_laser, LOW);
		laserEnabled = false;
	}
	else {
		digitalWrite(pin_laser, HIGH);
		laserEnabled = true;
	}
}

/* Initializes the Laser and the button */
void initGPIO() {
	//Deactivate the laser
	pinMode(pin_laser, OUTPUT);
	digitalWrite(pin_laser, LOW);
	laserEnabled = false;
	//Set button as input
	pinMode(pin_button, INPUT);
}

/* Disables all Chip-Select lines on the SPI bus */
void initSPI() {
	pinMode(pin_lcd_dc, OUTPUT);
	pinMode(pin_touch_cs, OUTPUT);
	pinMode(pin_flash_cs, OUTPUT);
	pinMode(pin_lepton_cs, OUTPUT);
	pinMode(pin_sd_cs, OUTPUT);
	pinMode(pin_lcd_cs, OUTPUT);
	digitalWrite(pin_lcd_dc, HIGH);
	digitalWrite(pin_touch_cs, HIGH);
	digitalWrite(pin_flash_cs, HIGH);
	digitalWrite(pin_lepton_cs, HIGH);
	digitalWrite(pin_sd_cs, HIGH);
	digitalWrite(pin_lcd_cs, HIGH);
	SPI.begin();
}

/* Inits the I2C Bus */
void initI2C() {
	//Start the Bus
	Wire.begin();
	//Enable Timeout for Hardware start
	Wire.setDefaultTimeout(1000);
	//Enable pullups
	Wire.pinConfigure(I2C_PINS_18_19, I2C_PULLUP_INT);
}

/* Init the Analog-Digital-Converter for the battery measure */
void initADC() {
	//Init ADC
	batMeasure = new ADC();
	//set number of averages
	batMeasure->setAveraging(4);
	//set bits of resolution
	batMeasure->setResolution(12);
	//change the conversion speed
	batMeasure->setConversionSpeed(ADC_MED_SPEED);
	//change the sampling speed
	batMeasure->setSamplingSpeed(ADC_MED_SPEED);
	//set battery pin as input
	pinMode(pin_bat_measure, INPUT);
}

/* Switch the clockline to the SD card */
void startAltClockline(bool sdStart) {
	CORE_PIN13_CONFIG = PORT_PCR_MUX(1);
	CORE_PIN14_CONFIG = PORT_PCR_DSE | PORT_PCR_MUX(2);
	if (sdStart)
		sd.begin(pin_sd_cs, SPI_FULL_SPEED);
}

/* Switch the clockline back to normal*/
void endAltClockline() {
	CORE_PIN13_CONFIG = PORT_PCR_DSE | PORT_PCR_MUX(2);
	CORE_PIN14_CONFIG = PORT_PCR_MUX(1);
}

/* Attach both interrupts */
void attachInterrupts() {
	//Attach the Button interrupt
	pinMode(pin_button, INPUT);
	attachInterrupt(pin_button, buttonIRQ, RISING);
	//Attach the Touch interrupt
	pinMode(pin_touch_irq, INPUT);
	attachInterrupt(pin_touch_irq, touchIRQ, FALLING);
}

/* Detach both interrupts */
void detachInterrupts() {
	//Detach the Button interrupt
	detachInterrupt(pin_button);
	//Detach the Touch interrupt
	detachInterrupt(pin_touch_irq);
}

/* A method to check if the touch screen is pressed */
bool touchScreenPressed() {
	//Check button status with debounce
	touchDebouncer.update();
	return touchDebouncer.read();
}


/* A method to check if the external button is pressed */
bool extButtonPressed() {
	//Check button status with debounce
	buttonDebouncer.update();
	return buttonDebouncer.read();
}

/* Disable the screen backlight */
void disableScreenLight() {
	digitalWrite(pin_lcd_backlight, LOW);
}

/* Enables the screen backlight */
void enableScreenLight() {
	digitalWrite(pin_lcd_backlight, HIGH);
}

/* Checks if the screen backlight is on or off*/
bool checkScreenLight() {
	return digitalRead(pin_lcd_backlight);
}

/* Checks the specific devic from the diagnostic variable */
bool checkDiagnostic(byte device) {
	return (diagnostic >> device) & 1;
}

/* Sets the status of a specific device from the diagnostic variable */
void setDiagnostic(byte device) {
	diagnostic &= ~(1 << device);
}

/* Prints the diagnostic infos on the serial console in case the display is not there*/

void printDiagnostic() {
	Serial.println("*** Diagnostic Infos ***");
	//Check spot sensor
	if (checkDiagnostic(diag_spot))
		Serial.println("Spot sensor - OK");
	else
		Serial.println("Spot sensor - Failed");
	//Check display
	if (checkDiagnostic(diag_display))
		Serial.println("Display - OK");
	else
		Serial.println("Display - Failed");
	//Check visual camera
	if (checkDiagnostic(diag_camera))
		Serial.println("Visual camera - OK");
	else
		Serial.println("Visual camera - Failed");
	//Check touch screen
	if (checkDiagnostic(diag_touch))
		Serial.println("Touch screen - OK");
	else
		Serial.println("Touch screen - Failed");
	//Check sd card
	if (checkDiagnostic(diag_sd))
		Serial.println("SD card - OK");
	else
		Serial.println("SD card - Failed");
	//Check battery gauge
	if (checkDiagnostic(diag_bat))
		Serial.println("Battery gauge - OK");
	else
		Serial.println("Battery gauge - Failed");
	//Check lepton config
	if (checkDiagnostic(diag_lep_conf))
		Serial.println("Lepton config - OK");
	else
		Serial.println("Lepton config - Failed");
	//Check lepton data
	if (checkDiagnostic(diag_lep_data))
		Serial.println("Lepton data - OK");
	else
		Serial.println("Lepton data - Failed");
}

/* Checks if the sd card is inserted for Early Bird Hardware */
bool checkSDCard() {
	//Old hardware, begin SD transaction
	if (mlx90614Version == mlx90614Version_old) {
		startAltClockline();
		if (!sd.begin(pin_sd_cs, SPI_FULL_SPEED)) {
			//Show error message if there is no card inserted
			drawMessage((char*) "Please insert SDCard!");
			delay(1000);
			//Go back 
			return false;
		}
		else
			return true;
		endAltClockline();
	}
	//All other do not need the check
	else
		return true;
}

/* Sets the display rotation depending on the setting */
void setDisplayRotation() {
	if (rotationEnabled) {
		display.setRotation(135);
		touch.setRotation(true);
	}
	else {
		display.setRotation(45);
		touch.setRotation(false);
	}
}

/* Initializes the display and checks if it is working */
void initDisplay() {
	//Init the display
	display.InitLCD();
	//Set the display rotation
	setDisplayRotation();
	//If not returning from mass storage, check display
	byte val = EEPROM.read(eeprom_massStorage);
	if ((val != eeprom_massStorage) && (val != eeprom_setValue)) {
		//Check status by writing test pixel red to 10/10
		display.setXY(10, 10, 10, 10);
		display.setPixel(VGA_RED);
		uint16_t color = display.readPixel(10, 10);
		if (color != VGA_RED)
			setDiagnostic(diag_display);
	}
	//Clear flag
	if(val == eeprom_massStorage)
		EEPROM.write(eeprom_massStorage, 0);
}

/* Initializes the touch module and checks if it is working */
void initTouch() {
	//Init the touch
	touch.begin();
	//If not capacitive, check if working
	if (!touch.capacitive) {
		TS_Point point = touch.getPoint();
		//No touch, working
		if ((point.x == 0) && (point.y == 0))
			return;
		//Not working
		else {
			drawMessage((char*) "Touch screen is not working!");
			delay(1000);
			setDiagnostic(diag_touch);
		}
	}
}

/* Resets the diagnostic status */
void resetDiagnostic() {
	diagnostic = diag_ok;
}

/* Checks for hardware issues */
void checkDiagnostic() {
	if (diagnostic != diag_ok) {
		//Show the diagnostics over serial
		printDiagnostic();
		//Show it on the screen
		showDiagnostic();
		//Wait for touch press
		while (!touch.touched());
		//Wait for touch release
		while (touch.touched());
	}
}

/* Checks if a FW upgrade has been done */
void checkFWUpgrade(){
	byte eepromVersion = EEPROM.read(eeprom_fwVersion);
	//Show message after firmware upgrade
	if (eepromVersion != fwVersion) {
		//Upgrade
		if (fwVersion > eepromVersion)
			drawMessage((char*)"FW update completed, pls restart!");
		//Downgrade
		else
			drawMessage((char*)"FW downgrade completed, pls restart!");
		//Set EEPROM firmware version to current one
		EEPROM.write(eeprom_fwVersion, fwVersion);
		while (true);
	}
}

/* Reads the old settings from EEPROM */
void readEEPROM() {
	byte read;
	//Temperature format
	read = EEPROM.read(eeprom_tempFormat);
	if ((read == tempFormat_celcius) || (read == tempFormat_fahrenheit))
		tempFormat = read;
	else
		tempFormat = tempFormat_celcius;
	//Color scheme
	read = EEPROM.read(eeprom_colorScheme);
	if ((read >= 0) && (read <= (colorSchemeTotal - 1)))
		colorScheme = read;
	else
		colorScheme = colorScheme_rainbow;
	//If Hot or Cold on startup, switch to Rainbow
	if ((colorScheme == colorScheme_coldest) || (colorScheme == colorScheme_hottest))
		colorScheme = colorScheme_rainbow;
	//Convert Enabled
	read = EEPROM.read(eeprom_convertEnabled);
	if ((read == false) || (read == true))
		convertEnabled = read;
	else
		convertEnabled = false;
	//Visual Enabled
	read = EEPROM.read(eeprom_visualEnabled);
	if ((read == false) || (read == true))
		visualEnabled = read;
	else
		visualEnabled = false;
	//Battery Enabled
	read = EEPROM.read(eeprom_batteryEnabled);
	if ((read == false) || (read == true))
		batteryEnabled = read;
	else
		batteryEnabled = false;
	//Time Enabled
	read = EEPROM.read(eeprom_timeEnabled);
	if ((read == false) || (read == true))
		timeEnabled = read;
	else
		timeEnabled = false;
	//Date Enabled
	read = EEPROM.read(eeprom_dateEnabled);
	if ((read == false) || (read == true))
		dateEnabled = read;
	else
		dateEnabled = false;
	//Points Enabled
	read = EEPROM.read(eeprom_pointsEnabled);
	if ((read == false) || (read == true))
		pointsEnabled = read;
	else
		pointsEnabled = false;
	//Storage Enabled
	read = EEPROM.read(eeprom_storageEnabled);
	if ((read == false) || (read == true))
		storageEnabled = read;
	else
		storageEnabled = false;
	//Spot Enabled
	read = EEPROM.read(eeprom_spotEnabled);
	if ((read == false) || (read == true))
		spotEnabled = read;
	else
		spotEnabled = true;
	//Filter Type
	read = EEPROM.read(eeprom_filterType);
	if ((read == filterType_none) || (read == filterType_box) || (read == filterType_gaussian))
		filterType = read;
	else
		filterType = filterType_box;
	//Colorbar Enabled
	read = EEPROM.read(eeprom_colorbarEnabled);
	if ((read == false) || (read == true))
		colorbarEnabled = read;
	else
		colorbarEnabled = true;
	//Rotation Enabled
	read = EEPROM.read(eeprom_rotationEnabled);
	if ((read == false) || (read == true))
		rotationEnabled = read;
	else
		rotationEnabled = false;
	//Display Mode
	read = EEPROM.read(eeprom_displayMode);
	if ((read == displayMode_thermal) || (read == displayMode_visual) || (read == displayMode_combined))
		displayMode = read;
	else
		displayMode = displayMode_thermal;
	//Return from Mass Storage reboot, no warmup required
	read = EEPROM.read(eeprom_massStorage);
	if (read == eeprom_setValue) {
		EEPROM.write(eeprom_massStorage, 0);
		calStatus = cal_standard;
	}
}

/* Startup procedure for the Hardware */
void initHardware() {
	//Start serial at 11.5k baud, does up to 12MBit/s
	Serial.begin(115200);
	//Init GPIO
	initGPIO();
	//SPI startup
	initSPI();
	//Init RTC
	setSyncProvider(getTeensy3Time);
	//Init I2C
	initI2C();
	//Init Display
	initDisplay();
	//Show Boot Screen
	bootScreen();
	//Init ADC
	initADC();
	//Init Camera module
	initCamera();
	//Init Touch screen
	initTouch();
	//Check battery for the first time
	checkBattery();
	//Init Lepton sensor
	initLepton();
	//Init Spot sensor
	mlx90614Init();
	//Init SD card
	initSD();
	//Disable I2C timeout
	Wire.setDefaultTimeout(0);
}
