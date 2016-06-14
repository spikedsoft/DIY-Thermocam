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
		imgSave = imgSave_set;
	//Long press - start video
	else {
		if (displayMode != displayMode_thermal) {
			drawMessage((char*) "Video only possible in thermal mode!");
			delay(1000);
		}
		else {
			detachInterrupts();
			videoSave = true;
		}
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
		if ((mlx90614Temp < min) && (colorScheme != colorScheme_hottest))
			min = mlx90614Temp;
		if ((mlx90614Temp > max) && (colorScheme != colorScheme_coldest))
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
	case colorScheme_arctic:
		colorMap = colorMap_arctic;
		colorElements = 240;
		break;
		//Black-Hot
	case colorScheme_blackHot:
		colorMap = colorMap_blackHot;
		colorElements = 224;
		break;
		//Blue-Red
	case colorScheme_blueRed:
		colorMap = colorMap_blueRed;
		colorElements = 192;
		break;
		//Coldest
	case colorScheme_coldest:
		colorMap = colorMap_coldest;
		colorElements = 224;
		break;
		//Contrast
	case colorScheme_contrast:
		colorMap = colorMap_contrast;
		colorElements = 224;
		break;
		//Double-Rainbow
	case colorScheme_doubleRainbow:
		colorMap = colorMap_doubleRainbow;
		colorElements = 256;
		break;
		//Gray-Red
	case colorScheme_grayRed:
		colorMap = colorMap_grayRed;
		colorElements = 224;
		break;
		//Glowbow
	case colorScheme_glowBow:
		colorMap = colorMap_glowBow;
		colorElements = 224;
		break;
		//Grayscale
	case colorScheme_grayscale:
		colorMap = colorMap_grayscale;
		colorElements = 256;
		break;
		//Hottest
	case colorScheme_hottest:
		colorMap = colorMap_hottest;
		colorElements = 224;
		break;
		//Ironblack
	case colorScheme_ironblack:
		colorMap = colorMap_ironblack;
		colorElements = 256;
		break;
		//Lava
	case colorScheme_lava:
		colorMap = colorMap_lava;
		colorElements = 240;
		break;
		//Medical
	case colorScheme_medical:
		colorMap = colorMap_medical;
		colorElements = 224;
		break;
		//Rainbow
	case colorScheme_rainbow:
		colorMap = colorMap_rainbow;
		colorElements = 256;
		break;
		//Wheel 1
	case colorScheme_wheel1:
		colorMap = colorMap_wheel1;
		colorElements = 256;
		break;
		//Wheel 2
	case colorScheme_wheel2:
		colorMap = colorMap_wheel2;
		colorElements = 256;
		break;
		//Wheel 3
	case colorScheme_wheel3:
		colorMap = colorMap_wheel3;
		colorElements = 256;
		break;
		//White-Hot
	case colorScheme_whiteHot:
		colorMap = colorMap_whiteHot;
		colorElements = 224;
		break;
		//Yellow
	case colorScheme_yellow:
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
	if ((colorScheme == colorScheme_coldest) || (colorScheme == colorScheme_hottest))
		hotColdChooser();
	//Save to EEPROM
	EEPROM.write(eeprom_colorScheme, colorScheme);
}

/* Lock or release limits */
void limitLock() {
	//If not warmed, do nothing
	if (calStatus == cal_warmup) {
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
	if (tempFormat == tempFormat_fahrenheit)
		mlx90614Temp = celciusToFahrenheit(mlx90614Temp);
	//Set text color, font and background
	display.setColor(VGA_WHITE);
	display.setBackColor(VGA_TRANSPARENT);
	display.setFont(smallFont);
	//If  not saving image or video
	if ((imgSave != imgSave_save) && (videoSave == false)) {
		//Show battery status in percantage
		if (batteryEnabled)
			displayBatteryStatus();
		//Show the time
		if (timeEnabled)
			displayTime();
		//Show the date
		if (dateEnabled)
			displayDate();
		//Show storage information
		if (storageEnabled)
			displayFreeSpace();
	}
	//Show the spot in the middle
	if (spotEnabled)
		showSpot();
	//Show the color bar when warmup is over and if enabled, not in visual mode
	if ((colorbarEnabled) && (calStatus > cal_warmup) && (displayMode != displayMode_visual))
		showColorBar();
	//Show the temperature points
	if (pointsEnabled)
		showTemperatures();
	//Activate the calibration after a warmup time of 60s
	if ((calStatus == cal_warmup) && (imgSave != imgSave_save) && (!videoSave)) {
		if (millis() - calTimer > 60000) {
			//Perform FFC if shutter is attached
			if (leptonVersion != leptonVersion_2_NoShutter)
				leptonRunCalibration();
			calStatus = cal_standard;
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
	//Change camera resolution
	if (displayMode == displayMode_thermal)
		changeCamRes(VC0706_640x480);
	else
		changeCamRes(VC0706_160x120);
	//Activate or deactivate combined mode
	if (displayMode != displayMode_combined)
		combinedDecomp = false;
	else
		combinedDecomp = true;
	//Attach the interrupts
	attachInterrupts();
	//Clear markers
	imgSave = false;
	videoSave = false;
	showMenu = false;
	lockLimits = false;
	//Allocate space
	image = (unsigned short*)calloc(19200, sizeof(unsigned short));
	showTemp = (uint16_t*)calloc(192, sizeof(uint16_t));
	//Clear showTemp values
	clearTemperatures();
}


/* Exit the live mode */
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
		//Show the content depending on the mode
		switch (displayMode) {
		//Thermal only
		case displayMode_thermal:
			displayThermalImg();
			break;
		//Visual only
		case displayMode_visual:
			displayVisualImg();
			break;
		//Combined
		case displayMode_combined:
			displayCombinedImg();
			break;
		}
		//Display additional information on the screen
		if (imgSave != imgSave_set)
			displayInfos();
		//Save the image
		if (imgSave == imgSave_save)
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