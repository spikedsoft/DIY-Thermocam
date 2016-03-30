/*
* Main functions in the live mode
*/

/* Includes */
#include "Calibration.h"
#include "Create.h"
#include "Load.h"
#include "Save.h"


/* If the touch has been pressed, enable menu */
void touchIRQ() {
	//When not in menu, show menu or lock/release limits
	if ((!showMenu) && (!videoSave)) {
		//Count the time to choose selection
		long startTime = millis();
		long endTime = millis() - startTime;
		//For capacitive touch
		if (touch.capacitive) {
			while ((touch.touched()) && (endTime <= 1000))
				endTime = millis() - startTime;
		}
		else {
			while ((!digitalRead(pin_touch_irq)) && (endTime <= 1000))
				endTime = millis() - startTime;
		}
		endTime = millis() - startTime;
		//Short press - show menu
		if (endTime < 1000)
			showMenu = true;
		//Long press - lock or release limits
		else {
			detachInterrupts();
			lockLimits = true;
		}
	}
}

/* Handler to check the external button and react to it */
void buttonIRQ() {
	//If we are in the video mode
	if (videoSave) {
		detachInterrupt(pin_button);
		videoSave = false;
		return;
	}
	//Count the time to choose selection
	long startTime = millis();
	long endTime = millis() - startTime;
	while ((extButtonPressed()) && (endTime <= 1000))
		endTime = millis() - startTime;
	endTime = millis() - startTime;
	//Short press - save image to SD Card
	if (endTime < 1000)
		//Prepare image save but let screen refresh first
		imgSave = 2;
	//Long press - start video
	else {
		detachInterrupts();
		videoSave = true;
	}
}

/* Show the color bar on screen */
void showColorBar() {
	//Set color
	display.setColor(VGA_WHITE);
	display.setBackColor(VGA_TRANSPARENT);
	//Help variables
	char buffer[6];
	byte red, green, blue;
	byte count = 0;
	byte height = 240 - ((240 - (colorElements / 2)) / 2);
	//Display color bar
	for (int i = 0; i < (colorElements - 1); i++) {
		if ((i % 2) == 0) {
			red = colorMap[i * 3];
			green = colorMap[(i * 3) + 1];
			blue = colorMap[(i * 3) + 2];
			display.setColor(red, green, blue);
			display.drawLine(285, height - count, 315, height - count);
			count++;
		}
	}
	//Get MLX90614 ambient temp
	mlx90614Amb = mlx90614GetAmb();
	//Calculate min and max
	float min = calFunction(minTemp);
	float max = calFunction(maxTemp);
	//Check if spot temp is out of range
	if ((agcEnabled) && (!limitsLocked)) {
		if ((mlx90614Temp < min) && (colorScheme != 8))
			min = mlx90614Temp;
		if ((mlx90614Temp > max) && (colorScheme != 3))
			max = mlx90614Temp;
	}
	//Calculate step
	float step = (max - min) / 3.0;
	//Draw min temp
	sprintf(buffer, "%d", (int)round(min));
	display.print(buffer, 260, height - 5);
	//Draw temperatures after min before max
	for (int i = 2; i >= 1; i--) {
		float temp = min + (i*step);
		sprintf(buffer, "%d", (int)round(temp));
		display.print(buffer, 260, height - 5 - (i * (colorElements / 6)));
	}
	//Draw max temp
	sprintf(buffer, "%d", (int)round(max));
	display.print(buffer, 260, height - 5 - (3 * (colorElements / 6)));
}

/* Show the current object temperature on screen*/
void showSpot() {
	//Draw the spot circle
	display.drawCircle(160, 120, 12);
	//Draw the lines
	display.drawHLine(136, 120, 12);
	display.drawHLine(172, 120, 12);
	display.drawVLine(160, 96, 12);
	display.drawVLine(160, 132, 12);
	//Convert to float with a special method
	char buffer[10];
	floatToChar(buffer, mlx90614Temp);
	display.print(buffer, 145, 150);
}

/* Map to the right color scheme */
void selectColorScheme() {
	//Select the right color scheme
	switch (colorScheme) {
		//Arctic
	case 0:
		colorMap = colorMap_arctic;
		colorElements = 240;
		break;
		//Black-Hot
	case 1:
		colorMap = colorMap_blackHot;
		colorElements = 224;
		break;
		//Blue-Red
	case 2:
		colorMap = colorMap_blueRed;
		colorElements = 192;
		break;
		//Coldest
	case 3:
		colorMap = colorMap_coldest;
		colorElements = 224;
		break;
		//Contrast
	case 4:
		colorMap = colorMap_contrast;
		colorElements = 224;
		break;
		//Double-Rainbow
	case 5:
		colorMap = colorMap_doubleRainbow;
		colorElements = 256;
		break;
		//Gray-Red
	case 6:
		colorMap = colorMap_grayRed;
		colorElements = 224;
		break;
		//Glowbow
	case 7:
		colorMap = colorMap_glowBow;
		colorElements = 224;
		break;
		//Hottest
	case 8:
		colorMap = colorMap_hottest;
		colorElements = 224;
		break;
		//Ironblack
	case 9:
		colorMap = colorMap_ironblack;
		colorElements = 256;
		break;
		//Lava
	case 10:
		colorMap = colorMap_lava;
		colorElements = 240;
		break;
		//Medical
	case 11:
		colorMap = colorMap_medical;
		colorElements = 224;
		break;
		//Rainbow
	case 12:
		colorMap = colorMap_rainbow;
		colorElements = 256;
		break;
		//Wheel 1
	case 13:
		colorMap = colorMap_wheel1;
		colorElements = 256;
		break;
		//Wheel 2
	case 14:
		colorMap = colorMap_wheel2;
		colorElements = 256;
		break;
		//Wheel 3
	case 15:
		colorMap = colorMap_wheel3;
		colorElements = 256;
		break;
		//White-Hot
	case 16:
		colorMap = colorMap_whiteHot;
		colorElements = 224;
		break;
		//Yellow
	case 17:
		colorMap = colorMap_yellow;
		colorElements = 224;
		break;
	}
}


/* Change the display options */
void changeDisplayOptions(byte* pos) {
	switch (*pos) {
		//Battery
	case 0:
		batteryEnabled = !batteryEnabled;
		EEPROM.write(eeprom_batteryEnabled, batteryEnabled);
		break;
		//Time
	case 1:
		timeEnabled = !timeEnabled;
		EEPROM.write(eeprom_timeEnabled, timeEnabled);
		break;
		//Date
	case 2:
		dateEnabled = !dateEnabled;
		EEPROM.write(eeprom_dateEnabled, dateEnabled);
		break;
		//Spot
	case 3:
		spotEnabled = !spotEnabled;
		EEPROM.write(eeprom_spotEnabled, spotEnabled);
		break;
		//Colorbar
	case 4:
		colorbarEnabled = !colorbarEnabled;
		EEPROM.write(eeprom_colorbarEnabled, colorbarEnabled);
		break;
		//Temperature Points
	case 5:
		pointsEnabled = !pointsEnabled;
		EEPROM.write(eeprom_pointsEnabled, pointsEnabled);
		break;
		//Storage
	case 6:
		storageEnabled = !storageEnabled;
		EEPROM.write(eeprom_storageEnabled, storageEnabled);
		break;
		//Filter
	case 7:
		filterEnabled = !filterEnabled;
		EEPROM.write(eeprom_filterEnabled, filterEnabled);
		break;
	}
}


/* Change the color scheme for the thermal image */
void changeColorScheme(byte* pos) {
	//Align position to color scheme
	colorScheme = *pos;
	//Map to the right color scheme
	selectColorScheme();
	//Choose limits for hot and cold mode
	if ((colorScheme == 3) || (colorScheme == 8))
		hotColdChooser();
	//Save to EEPROM
	EEPROM.write(eeprom_colorScheme, colorScheme);
}

/* Lock or release limits */
void limitLock() {
	//If not warmed, do nothing
	if (calStatus == 0) {
		showMsg((char*) "Wait for warmup");
	}
	//Unlock limits
	else if (limitsLocked) {
		showMsg((char*) "Limits unlocked");
		limitsLocked = false;
	}
	//Lock limits
	else {
		showMsg((char*) "Limits locked");
		limitsLocked = true;
	}
	attachInterrupts();
	showMenu = false;
	lockLimits = false;
}

/* Display addition information on the screen */
void displayInfos() {
	///Refresh object temperature
	mlx90614GetTemp();
	//Convert to Fahrenheit if needed
	if (tempFormat)
		mlx90614Temp = celciusToFahrenheit(mlx90614Temp);
	//Set text color, font and background
	display.setColor(VGA_WHITE);
	display.setBackColor(VGA_TRANSPARENT);
	display.setFont(smallFont);
	//Show battery status in percantage
	if ((batteryEnabled) && (imgSave != 1) && (!videoSave))
		displayBatteryStatus();
	//Show the time
	if ((timeEnabled) && (imgSave != 1) && (!videoSave))
		displayTime();
	//Show the date
	if ((dateEnabled) && (imgSave != 1) && (!videoSave))
		displayDate();
	//Show storage information
	if ((storageEnabled) && (imgSave != 1) && (!videoSave))
		displayFreeSpace();
	//Show the spot in the middle
	if (spotEnabled)
		showSpot();
	//Show the color bar when warmup is over and if enabled
	if ((colorbarEnabled) && (calStatus > 0))
		showColorBar();
	//Show the temperature points
	if (pointsEnabled)
		showTemperatures();
	//Activate the calibration after a warmup time of 60s
	if ((calStatus == 0) && (imgSave != 1) && (!videoSave)) {
		if (millis() - calTimer > 60000) {
			//Perform FFC if shutter is attached
			if (leptonVersion != 2)
				leptonRunCalibration();
			calStatus = 1;
		}
		else
			displayWarmup();
	}
}

/* Init procedure for the live mode */
void liveModeInit() {
	//Activate laser if enabled
	if (laserEnabled)
		digitalWrite(pin_laser, HIGH);
	//Select color scheme
	selectColorScheme();
	//Attach the interrupts
	attachInterrupts();
	//Allocate space
	image = (unsigned short*)calloc(19200, sizeof(unsigned short));
	showTemp = (uint16_t*)calloc(192, sizeof(uint16_t));
	//Clear showTemp values
	clearTemperatures();
}

void liveModeExit() {
	//Deactivate laser if enabled
	if (laserEnabled)
		digitalWrite(pin_laser, LOW);
	//Detach the interrupts
	detachInterrupts();
	//Deallocate space
	free(image);
	free(showTemp);
	//Open the main menu
	mainMenu();
}

/* Main entry point for the live mode */
void liveMode() {
	//Init
	liveModeInit();
	//Main Loop
	while (true) {
		//If touch IRQ has been triggered, open menu
		if (showMenu) {
			if (liveMenu())
				break;
		}
		//Create and display the thermal image
		displayThermalImg();
		//Display additional information on the screen
		if (imgSave != 2)
			displayInfos();
		//Save the image
		if (imgSave == 1)
			saveImage();
		//Start the video
		if (videoSave) {
			if (videoModeChooser())
				videoCapture();
		}
		//Release or lock the limits
		if (lockLimits)
			limitLock();
	}
	//Exit
	liveModeExit();
}