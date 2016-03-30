/*
* Main hardware functions
*/

/* Includes */
#include "Battery.h"
#include "Cam.h"
#include "MLX90614.h"
#include "Lepton.h"
#include "Storage.h"
#include "Connection.h"


/* Methods */

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

/* Checks if the sd card is inserted for Early Bird Hardware */
bool checkSDCard() {
	//Early-Bird #1
	if (mlx90614Version == 0) {
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

/* Reads the old settings from EEPROM */
void readEEPROM() {
	byte read;
	//Read settings if first start is done
	if ((EEPROM.read(eeprom_firstStart) == eeprom_setValue) || (EEPROM.read(eeprom_firstStart) == 100)) {
		//Temperature format
		read = EEPROM.read(eeprom_tempFormat);
		if ((read == 0) || (read == 1))
			tempFormat = read;
		//Color scheme
		read = EEPROM.read(eeprom_colorScheme);
		if ((read >= 0) && (read <= 17))
			colorScheme = read;
		//If Hot or Cold on startup, switch to Rainbow
		if ((colorScheme == 3) || (colorScheme == 8))
			colorScheme = 12;
		//Convert Enabled
		read = EEPROM.read(eeprom_convertEnabled);
		if ((read == 0) || (read == 1))
			convertEnabled = read;
		//Visual Enabled
		read = EEPROM.read(eeprom_visualEnabled);
		if ((read == 0) || (read == 1))
			visualEnabled = read;
		//Battery Enabled
		read = EEPROM.read(eeprom_batteryEnabled);
		if ((read == 0) || (read == 1))
			batteryEnabled = read;
		//Time Enabled
		read = EEPROM.read(eeprom_timeEnabled);
		if ((read == 0) || (read == 1))
			timeEnabled = read;
		//Date Enabled
		read = EEPROM.read(eeprom_dateEnabled);
		if ((read == 0) || (read == 1))
			dateEnabled = read;
		//Points Enabled
		read = EEPROM.read(eeprom_pointsEnabled);
		if ((read == 0) || (read == 1))
			pointsEnabled = read;
		//Storage Enabled
		read = EEPROM.read(eeprom_storageEnabled);
		if ((read == 0) || (read == 1))
			storageEnabled = read;
		//Spot Enabled
		read = EEPROM.read(eeprom_spotEnabled);
		if ((read == 0) || (read == 1))
			spotEnabled = read;
		//Filter Enabled
		read = EEPROM.read(eeprom_filterEnabled);
		if ((read == 0) || (read == 1))
			filterEnabled = read;
		//Colorbar Enabled
		read = EEPROM.read(eeprom_colorbarEnabled);
		if ((read == 0) || (read == 1))
			colorbarEnabled = read;
		//Rotation Enabled
		read = EEPROM.read(eeprom_rotationEnabled);
		if ((read == 0) || (read == 1))
			rotationEnabled = read;
		//Return from Mass Storage reboot, no warmup required
		read = EEPROM.read(eeprom_massStorage);
		if (read == eeprom_setValue) {
			EEPROM.write(eeprom_massStorage, 0);
			calStatus = 1;
		}
	}
	//Do first start
	else
		firstStart();
	//Show hint screen
	if (EEPROM.read(eeprom_firstStart) == 100)
		liveModeHelper();
}

/* Startup procedure for the Hardware */
void initHardware() {
	//Laser off
	pinMode(pin_laser, OUTPUT);
	digitalWrite(pin_laser, LOW);
	//SPI startup
	initSPI();
	//Init Time
	setTime(Teensy3Clock.get());
	//Set SD Timestamp to current time
	SdFile::dateTimeCallback(dateTime);
	//Init I2C
	initI2C();
	//Init the MLX90614
	mlx90614Init();
	//Init Display
	display.InitLCD();
	//Show Boot Screen
	bootScreen();
	//Push Button
	pinMode(pin_button, INPUT);
	//Init ADC
	initADC();
	//Init Camera module
	initCamera();
	//Init Touch screen
	touch.begin();
	//Init SD card
	initSD();
	//Check battery for the first time
	checkBattery();
	//Check Lepton HW Revision
	initLepton();
	//Disable I2C timeout
	Wire.setDefaultTimeout(0);
	//Read EEPROM settings
	readEEPROM();
	//Set rotation
	setDisplayRotation();
}
