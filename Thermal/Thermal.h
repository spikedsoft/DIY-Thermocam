/*
* Main functions in the live mode
*/

/* Includes */
#include "Create.h"
#include "Load.h"
#include "Save.h"

/* If the touch has been pressed, enable menu */
void touchIRQ() {
	//When in video mode, toggle display
	if (videoSave == true) {
		bool displayState = digitalRead(pin_lcd_backlight);
		if (displayState == false)
			displayOn(true);
		else
			displayOff(true);
		return;
	}
	//When not in video mode, show menu
	if (showMenu == false)
		showMenu = true;
}

/* Handler to check the external button and react to it */
void buttonIRQ() {
	//Check if the Button is pressed
	if (extButtonPressed()) {
		//If we are in the video mode
		if (videoSave) {
			detachInterrupt(pin_button);
			videoSave = false;
			return;
		}
		//For Early Bird HW, check if the SD card is there
		if (mlx90614Version == 0) {
			if (!checkSDCard())
				return;
		}
		//Check if there is at least 1MB of space left
		if (getSDSpace() < 1000) {
			drawMessage((char*) "Not enough space, return..");
			delay(1000);
			return;
		}
		//Count the time to choose selection
		long startTime = millis();
		long endTime = millis() - startTime;
		while ((extButtonPressed()) && (endTime <= 1000))
			endTime = millis() - startTime;
		endTime = millis() - startTime;
		//Short press - save image to SD Card
		if (endTime < 1000) {
			//Show message on screen
			showMsg((char*) "Save Thermal..");
			//Prepare image save but let screen refresh first
			imgSave = 2;
			delay(500);
		}
		//Long press - start video
		else {
			detachInterrupts();
			videoSave = true;
		}
	}
}

/* Converts a given Temperature in Celcius to Fahrenheit */
float celciusToFahrenheit(float Tc) {
	float Tf = ((float) 9.0 / 5.0) * Tc + 32.0;
	return (Tf);
}

/* Function to calculate temperature out of Lepton value */
float calFunction(uint16_t rawValue) {
	float temp;
	//Refresh offset after 10 seconds in case ambient temp changed
	if ((millis() - refreshTime) > 10000) {
		calOffset = mlx90614GetAmb();
		//If ambient temp changed more than 3 degress, trigger calibration
		if (abs(calOffset - calOffset_old) > 3.0)
			leptonRunCalibration();
		calOffset_old = calOffset;
		refreshTime = millis();
	}
	temp = (0.025 * (rawValue - 8192)) + calOffset;
	//Convert to Fahrenheit if needed
	if (tempFormat)
		temp = celciusToFahrenheit(temp);
	return temp;
}

/* Show the color bar on screen */
void showColorBar() {
	byte red, green, blue;
	byte count = 0;
	for (int i = 0; i < 255; i++) {
		if ((i % 2) == 0) {
			//Cold
			if (colorScheme == 3) {
				if (i < (255 - grayscaleLevel))
					colormap = colormap_grayscale;
				else
					colormap = colormap_rainbow;
			}
			//Cold
			if (colorScheme == 4) {
				if (i > grayscaleLevel)
					colormap = colormap_grayscale;
				else
					colormap = colormap_rainbow;
			}
			red = colormap[i * 3];
			green = colormap[(i * 3) + 1];
			blue = colormap[(i * 3) + 2];
			display.setColor(red, green, blue);
			display.drawLine(285, 184 - count, 315, 184 - count);
			count++;
		}
	}
	float min = calFunction(minTemp);
	float max = calFunction(maxTemp);
	float step = (max - min) / 4.0;
	//Set color
	setColor();
	display.setBackColor(VGA_TRANSPARENT);
	//Draw temperatures
	char buffer[6];
	for (int i = 0; i < 5; i++) {
		float temp = max - (i*step);
		sprintf(buffer, "%d", (int)temp);
		display.print(buffer, 260, 51 + (i * 32));
	}
}

/* Toggles the colorbar */
void toggleColorbar() {
	if (colorbarEnabled) {
		colorbarEnabled = false;
	}
	else {
		colorbarEnabled = true;
	}
	EEPROM.write(eeprom_colorbarEnabled, colorbarEnabled);
}

/* Show the current object temperature on screen*/
void showSpot(bool save) {
	//Set text color
	setColor();
	display.setBackColor(VGA_TRANSPARENT);
	//Draw the spot circle
	display.drawCircle(160, 120, 12);
	//Draw the lines
	display.drawHLine(136, 120, 12);
	display.drawHLine(172, 120, 12);
	display.drawVLine(160, 96, 12);
	display.drawVLine(160, 132, 12);
	//Receive object temperature
	if (!save) {
		mlx90614GetTemp();
		//Convert to Fahrenheit if needed
		if (tempFormat == 1)
			mlx90614Temp = celciusToFahrenheit(mlx90614Temp);
	}
	//Convert to float with a special method
	char buffer[10];
	floatToChar(buffer, mlx90614Temp);
	display.print(buffer, 145, 150);
}


/* Toggles the spot display */
void toggleSpot() {
	if (spotEnabled) {
		spotEnabled = false;
	}
	else {
		spotEnabled = true;
	}
	//Save to EEPROM
	EEPROM.write(eeprom_spotEnabled, spotEnabled);
}

/* Toggles the filter */
void toggleFilter() {
	if (filterEnabled) {
		filterEnabled = false;
	}
	else {
		filterEnabled = true;
	}
	//Save to EEPROM
	EEPROM.write(eeprom_filterEnabled, filterEnabled);
}

/* Map to the right color scheme */
void selectColorScheme() {
	//Select the right color scheme
	if (colorScheme == 0)
		colormap = colormap_rainbow;
	else if (colorScheme == 1)
		colormap = colormap_ironblack;
	else
		colormap = colormap_grayscale;
}

/* Change the color scheme for the thermal image */
void changeColorScheme(int pos) {
	switch (pos) {
		//Rainbow
	case 0:
		colorScheme = 0;
		break;
		//Ironblack
	case 1:
		colorScheme = 1;
		break;
		//Grayscale
	case 2:
		colorScheme = 2;
		break;
		//Hot
	case 3:
		colorScheme = 3;
		break;
		//Cold
	case 4:
		colorScheme = 4;
		break;
	}
	//Map to the right color scheme
	selectColorScheme();
	//Choose limits for hot and cold mode
	if ((colorScheme == 3) || (colorScheme == 4))
		hotColdChooser();
	//Save to EEPROM
	EEPROM.write(eeprom_colorScheme, colorScheme);
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
}

void liveModeExit() {
	//Deactivate laser if enabled
	if (laserEnabled)
		digitalWrite(pin_laser, LOW);
	//Detach the interrupts
	detachInterrupts();
	//Deallocate space
	free(image);
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
			//Detach the interrupts
			detachInterrupts();
			//Show the Menu
			int ret = liveMenu();
			showMenu = false;
			if (ret)
				break;
			//Re-attach the interrupts
			attachInterrupts();
		}
		//Create and display the thermal image
		displayThermalImg();
		//Show the spot in the middle
		if (spotEnabled)
			showSpot();
		//Show the color bar
		if (colorbarEnabled)
			showColorBar();
		//Save the image
		if (imgSave == 1) {
			//Detach the interrupts
			detachInterrupts();
			saveImage();
			imgSave = false;
			//Re-attach the interrupts
			attachInterrupts();
		}
		//Start the video
		if (videoSave == 1) {
			//Ask user for the video interval
			if (videoIntervalChooser())
				videoCapture();
			//Re-attach the interrupts
			attachInterrupts();
			//Disable mode
			videoSave = false;
			imgSave = false;
		}
		//Save screenshot if serial command is send
		//saveScreenshot();
	}
	//Exit
	liveModeExit();
}