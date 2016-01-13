/*
* Main hardware functions
*/

/* Includes */
#include "Battery.h"
#include "Cam.h"
#include "MLX90614.h"
#include "Lepton.h"
#include "Storage.h"


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
	Wire.begin();
	//Early-Bird #1
	if(mlx90614Version == 0)
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

/* Turn display off*/
void displayOff(bool save) {
	detachInterrupt(pin_touch_irq);
	if (!save) {
		drawMessage((char*) "Press touch to reactivate display!");
		delay(1000);
		display.enterSleepMode();
	}
	digitalWrite(pin_lcd_backlight, LOW);
	while (touch.touched());
	delay(100);
	attachInterrupt(pin_touch_irq, touchIRQ, FALLING);
}

/* Turn display on*/
void displayOn(bool save) {
	detachInterrupt(pin_touch_irq);
	digitalWrite(pin_lcd_backlight, HIGH);
	if (!save) {
		display.exitSleepMode();
		drawMessage((char*) "Turning display on..");
		delay(1000);
	}
	while (touch.touched());
	delay(100);
	attachInterrupt(pin_touch_irq, touchIRQ, FALLING);
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

/* A method to check if the external button is pressed */
bool extButtonPressed() {
	//Check button status with debounce
	buttonDebouncer.update();
	return buttonDebouncer.read();
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

/* Reads the old settings from EEPROM */
bool checkEEPROM() {
	byte read;
	//Read settings if first start is done
	if ((EEPROM.read(eeprom_firstStart) == eeprom_setValue) || (EEPROM.read(eeprom_firstStart) == 100)) {
		//Temperature format
		read = EEPROM.read(eeprom_tempFormat);
		if ((read == 0) || (read == 1))
			tempFormat = read;
		//Color Scheme
		read = EEPROM.read(eeprom_imagesFormat);
		if ((read == 0) || (read == 1))
			imagesFormat = read;
		//Images Type
		read = EEPROM.read(eeprom_imagesType);
		if ((read == 0) || (read == 1))
			imagesType = read;
		//Videos Format
		read = EEPROM.read(eeprom_videosFormat);
		if ((read == 0) || (read == 1))
			videosFormat = read;
		//Videos Type
		read = EEPROM.read(eeprom_videosType);
		if ((read == 0) || (read == 1))
			videosType = read;
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
		//Return from Mass Storage reboot
		read = EEPROM.read(eeprom_massStorage);
		if (read == eeprom_setValue) {
			EEPROM.write(eeprom_massStorage, 0);
			return true;
		}
	}
	//Do first start
	else
		firstStart();
	//Show hint screen
	if (EEPROM.read(eeprom_firstStart) == 100)
		liveModeHelper();
	return false;
}

/* Startup procedure for the Hardware */
void initHardware() {
	//Serial debugging
	//Serial.begin(115200);
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
	//Init Display
	display.InitLCD();
	//Show Boot Screen
	bootScreen();
	//Push Button
	pinMode(pin_button, INPUT);
	//Init ADC
	initADC();
	//Init Touch
	touch.begin();
	//Init the MLX90614
	mlx90614Init();
	//Init Camera module
	initCamera();
	//Init SD card
	initSD();
	//Check battery for the first time
	checkBattery(true);
	//Check Lepton HW Revision
	initLepton();
}
