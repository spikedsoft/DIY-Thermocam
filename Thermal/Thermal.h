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
	//Help variables
	char buffer[6];
	byte red, green, blue;
	byte count = 0;
	byte height = 120 - ((120 - (colorElements / 4)) / 2);
	//Display color bar
	for (int i = 0; i < (colorElements - 1); i++) {
		if ((i % 4) == 0) {
			red = colorMap[i * 3];
			green = colorMap[(i * 3) + 1];
			blue = colorMap[(i * 3) + 2];
			display.setColor(red, green, blue);
			display.drawLine(142, height - count, 157, height - count);
			count++;
		}
	}
	//Switch color back
	display.setColor(VGA_WHITE);
	//Calculate min and max temp in celcius/fahrenheit
	float min = calFunction(minTemp);
	float max = calFunction(maxTemp);
	//Calculate step
	float step = (max - min) / 3.0;
	//Draw min temp
	sprintf(buffer, "%d", (int)round(min));
	display.print(buffer, 262, (height * 2) - 5);
	//Draw temperatures after min before max
	for (int i = 2; i >= 1; i--) {
		float temp = min + (i*step);
		sprintf(buffer, "%d", (int)round(temp));
		display.print(buffer, 262, (height * 2) - 5 - (i * (colorElements / 6)));
	}
	//Draw max temp
	sprintf(buffer, "%d", (int)round(max));
	display.print(buffer, 262, (height * 2) - 5 - (3 * (colorElements / 6)));
}

/* Show the current object temperature on screen*/
void showSpot() {
	//Draw the spot circle
	display.drawCircle(80, 60, 6);
	//Draw the lines
	display.drawLine(68, 60, 74, 60);
	display.drawLine(86, 60, 92, 60);
	display.drawLine(80, 48, 80, 54);
	display.drawLine(80, 66, 80, 72);
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
		if (filterType == filterType_box)
			filterType = filterType_gaussian;
		else if (filterType == filterType_gaussian)
			filterType = filterType_none;
		else
			filterType = filterType_box;
		EEPROM.write(eeprom_filterType, filterType);
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

/* Show the thermal/visual/combined image on the screen */
void showImage() {
	//Draw thermal image on screen if created previously and not in menu
	if ((imgSave != imgSave_set) && (showMenu == false))
		display.writeScreen(image);
	//If the image has been created, set to save
	if (imgSave == imgSave_create)
		imgSave = imgSave_save;
}

/* Display addition information on the screen */
void displayInfos() {
	//Set text color, font and background
	display.setColor(VGA_WHITE);
	display.setBackColor(VGA_TRANSPARENT);
	display.setFont(tinyFont);

	//Set write to image, not display
	display.writeToImage = true;
	//Check warmup
	checkWarmup();

	//If  not saving image or video
	if ((imgSave != imgSave_create) && (videoSave == false)) {
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
		//Display warmup if required
		if ((videoSave == false) && (calStatus == cal_warmup))
			displayWarmup();
	}

	//Show the spot in the middle
	if (spotEnabled)
		showSpot();
	//Show the color bar when warmup is over and if enabled, not in visual mode
	if ((colorbarEnabled) && (calStatus != cal_warmup) && (displayMode != displayMode_visual))
		showColorBar();
	//Show the temperature points
	if (pointsEnabled)
		showTemperatures();
	//Set write back to display
	display.writeToImage = false;
}

/* Check for serial connection */
void checkSerial() {
	//Check for incoming serial data
	if ((Serial.available() > 0) && (Serial.read() == CMD_START))
		serialConnect();
	//Another command received, discard it
	else if ((Serial.available() > 0))
		Serial.read();
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
	//Open the main menu
	mainMenu();
}

/* Main entry point for the live mode */
void liveMode() {
	//Init
	liveModeInit();
	//Main Loop
	while (true) {
		//Check for serial connection
		checkSerial();

		//If touch IRQ has been triggered, open menu
		if (showMenu) {
			if (liveMenu())
				break;
		}

		//Check if the save message needs to be shown
		if (imgSave == imgSave_set)
			showSaveMessage();

		//Create the content depending on the mode
		switch (displayMode) {
			//Thermal only
		case displayMode_thermal:
			createThermalImg();
			break;
			//Visual only
		case displayMode_visual:
			createVisualImg();
			break;
			//Combined
		case displayMode_combined:
			createCombinedImg();
			break;
		}

		//Display additional information
		if (imgSave != imgSave_set)
			displayInfos();

		//Show the content on the screen
			showImage();

		//Save the converted / visual image
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