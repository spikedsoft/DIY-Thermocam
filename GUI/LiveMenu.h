/*
* Menu inside the live mode
*/

/* Draws the background in the live menu */
void liveMenuBackground() {
	display.setColor(127, 127, 127);
	display.fillRoundRect(6, 6, 314, 234);
	display.setColor(153, 162, 163);
	display.fillRect(6, 36, 314, 180);
	display.setColor(VGA_BLACK);
	display.drawHLine(6, 36, 314);
	display.drawHLine(6, 180, 314);
}

/* Draws the title in the live menu */
void liveMenuTitle(char* title) {
	display.setFont(bigFont);
	display.setBackColor(126, 126, 126);
	display.setColor(VGA_WHITE);
	display.print(title, CENTER, 14);
}

/* Draws the current selection in the menu */
void liveMenuSelection(char* selection) {
	//Clear the old content
	display.setColor(VGA_WHITE);
	display.fillRect(66, 58, 257, 111);
	//Print the text
	display.setBackColor(VGA_WHITE);
	display.setColor(255, 106, 0);
	display.print(selection, CENTER, 77);
}

/* Calibration*/
void calibrationScreen() {
	//Title & Background
	liveMenuBackground();
	liveMenuTitle((char*)"Calibrating..");
	display.setColor(VGA_WHITE);
	display.setBackColor(153, 162, 163);
	display.setFont(smallFont);
	display.print((char*)"Point the camera to different", CENTER, 63);
	display.print((char*)"hot and cold object in the area.", CENTER, 96);
	touchButtons.deleteAllButtons();
	touchButtons.setTextFont(bigFont);
	touchButtons.addButton(90, 188, 140, 40, (char*) "Abort");
	touchButtons.drawButtons();
	display.setFont(bigFont);
	display.print((char*) "Status:  0%", CENTER, 140);
}

/* Calibration Repeat Choose */
bool calibrationRepeat() {
	//Title & Background
	liveMenuBackground();
	liveMenuTitle((char*)"Bad Calibration");
	display.setColor(VGA_WHITE);
	display.setFont(bigFont);
	display.setBackColor(153, 162, 163);
	display.print((char*)"Try again ?", CENTER, 66);
	display.setFont(smallFont);
	display.setBackColor(127, 127, 127);
	display.print((char*)"Use different calibration objects !", CENTER, 201);
	//Draw the buttons
	touchButtons.deleteAllButtons();
	touchButtons.setTextFont(bigFont);
	touchButtons.addButton(15, 106, 140, 55, (char*) "No");
	touchButtons.addButton(165, 106, 140, 55, (char*) "Yes");
	touchButtons.drawButtons();
	//Touch handler
	while (true) {
		//If touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//YES
			if (pressedButton == 1) {
				return true;
				break;
			}
			//NO
			else if (pressedButton == 0) {
				return false;
				break;
			}
		}
	}
	return true;
}

/* Calibration Chooser */
bool calibrationChooser() {
	//Title & Background
	liveMenuBackground();
	liveMenuTitle((char*)"Calibration");
	//Draw the buttons
	touchButtons.deleteAllButtons();
	touchButtons.setTextFont(bigFont);
	touchButtons.addButton(15, 47, 140, 120, (char*) "New");
	touchButtons.addButton(165, 47, 140, 120, (char*) "Delete");
	touchButtons.addButton(15, 188, 140, 40, (char*) "Back");
	touchButtons.drawButtons();
	//Touch handler
	while (true) {
		//If touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//NEW
			if (pressedButton == 0) {
				calibrationProcess();
				return true;
				break;
			}
			//DELETE
			else if (pressedButton == 1) {
				calSlope = 0.025;
				calStatus = 1;
				return true;
				break;
			}
			//BACK
			else if (pressedButton == 2)
				return false;
		}
	}
	return true;
}

/* Touch handler for the hot & cold limit changer menu */
void hotColdChooserHandler() {
	grayscaleLevel = 85;
	byte grayscaleLevel_old = grayscaleLevel;
	char margin[14];
	float scale;
	uint16_t value;
	int temp = 0;
	while (1) {
		display.setFont(smallFont);
		scale = 255.0 / (maxTemp - minTemp);
		value = (grayscaleLevel / scale) + minTemp;
		temp = (int)calFunction(value);
		//Display Fahrenheit or Celcius
		if (!tempFormat) {
			sprintf(margin, "Margin: %dC", temp);
		}
		else {
			sprintf(margin, "Margin: %dF", temp);
		}
		display.print(margin, CENTER, 145);
		display.setFont(bigFont);
		//If touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//RESET
			if (pressedButton == 0) {
				//Restore the old values
				grayscaleLevel = grayscaleLevel_old;
			}
			//SELECT
			else if (pressedButton == 1) {
				break;
			}
			//MINUS
			else if (pressedButton == 2) {
				while ((((int)calFunction(value)) > (temp - 1)) && (grayscaleLevel > 1)) {
					grayscaleLevel -= 1;
					value = (grayscaleLevel / scale) + minTemp;
				}
			}
			//PLUS
			else if (pressedButton == 3) {
				while ((((int)calFunction(value)) < (temp + 1)) && (grayscaleLevel < 255)) {
					grayscaleLevel += 1;
					value = (grayscaleLevel / scale) + minTemp;
				}
			}
			//Prepare the preview image
			delay(10);
			createThermalImg(true);
			//Display the preview image
			display.drawBitmap(80, 40, 160, 120, image, 1);
		}
	}
}

/* Select the limit in Hot & Cold mode */
void hotColdChooser() {
	//Background & title
	liveMenuBackground();
	liveMenuTitle((char*) "Set Level");
	//Draw the buttons
	touchButtons.deleteAllButtons();
	touchButtons.setTextFont(bigFont);
	touchButtons.addButton(15, 188, 140, 40, (char*) "Reset");
	touchButtons.addButton(165, 188, 140, 40, (char*) "OK");
	touchButtons.addButton(15, 48, 55, 120, (char*) "-");
	touchButtons.addButton(250, 48, 55, 120, (char*) "+");
	touchButtons.drawButtons();
	//Prepare the preview image
	delay(10);
	createThermalImg(true);
	//Display the preview image
	display.drawBitmap(80, 40, 160, 120, image, 1);
	//Draw the border for the preview image
	display.setColor(VGA_WHITE);
	display.drawRect(79, 39, 241, 161);
	//Go into the normal touch handler
	hotColdChooserHandler();
}

/* Touch Handler for the limit chooser menu */
void limitChooserHandler() {
	//Save the old limits in case the user wants to restore them
	uint16_t maxTemp_old = maxTemp;
	uint16_t minTemp_old = minTemp;
	//Set both modes to false for the first time
	bool minChange = false;
	bool maxChange = false;
	//Buffer
	int currentVal, min, max;
	char minC[10];
	char maxC[10];
	//Touch handler
	while (1) {
		display.setFont(smallFont);
		min = (int)calFunction(minTemp);
		max = (int)calFunction(maxTemp);
		if (!tempFormat) {
			sprintf(minC, "Min:%dC", min);
			sprintf(maxC, "Max:%dC", max);
		}
		else {
			sprintf(minC, "Min:%dF", min);
			sprintf(maxC, "Max:%dF", max);
		}
		display.print(maxC, 180, 145);
		display.print(minC, 85, 145);
		display.setFont(bigFont);
		//If touch pressed
		if (touch.touched() == true) {
			int pressedButton;
			//Change values continously when the user holds the plus or minus button
			if (minChange || maxChange)
				pressedButton = touchButtons.checkButtons(true);
			//Normal check when not in minChange or maxChange mode
			else
				pressedButton = touchButtons.checkButtons();
			//RESET
			if (pressedButton == 0) {
				//Restore the old values
				if (minChange) {
					minTemp = minTemp_old;
				}
				else if (maxChange) {
					maxTemp = maxTemp_old;
				}
				else {
					minTemp = minTemp_old;
					maxTemp = maxTemp_old;
				}
			}
			//SELECT
			else if (pressedButton == 1) {
				//Leave the minimum or maximum change mode
				if (minChange || maxChange) {
					touchButtons.relabelButton(1, (char*) "OK", true);
					touchButtons.relabelButton(2, (char*) "Min", true);
					touchButtons.relabelButton(3, (char*) "Max", true);
					if (minChange == true)
						minChange = false;
					if (maxChange == true)
						maxChange = false;
				}
				//Go back to the create image menu
				else {
					break;
				}
			}
			//DECREASE
			else if (pressedButton == 2) {
				//In minimum change mode - decrease minimum temp
				if ((minChange == true) && (maxChange == false)) {
					currentVal = (int)calFunction(minTemp);
					//Check if minimum temp is not already too low
					if (currentVal > -69.00) {
						while (((int)calFunction(minTemp)) > (currentVal - 1))
							minTemp = minTemp - 1;
					}
				}
				//Enter minimum change mode
				else if ((minChange == false) && (maxChange == false)) {
					touchButtons.relabelButton(1, (char*) "Back", true);
					touchButtons.relabelButton(2, (char*) "-", true);
					touchButtons.relabelButton(3, (char*) "+", true);
					minChange = true;
				}
				//in maximum change mode - decrease maximum temp
				else if ((minChange == false) && (maxChange == true)) {
					currentVal = (int)calFunction(maxTemp);
					//Check of maximum temp is still biggerer than minimum temp
					if (currentVal > ((int)calFunction(minTemp) + 1)) {
						while (((int)calFunction(maxTemp)) > (currentVal - 1))
							maxTemp = maxTemp - 1;
					}
				}
			}
			//INCREASE
			else if (pressedButton == 3) {
				//In maximum change mode - increase maximum temp
				if ((minChange == false) && (maxChange == true)) {
					currentVal = (int)calFunction(maxTemp);
					//Check if maximum temp is not already too high
					if (currentVal < 379.00) {
						while (((int)calFunction(maxTemp)) < (currentVal + 1))
							maxTemp = maxTemp + 1;
					}
				}
				//Enter maximum change mode
				else if ((minChange == false) && (maxChange == false)) {
					touchButtons.relabelButton(1, (char*) "Back", true);
					touchButtons.relabelButton(2, (char*) "-", true);
					touchButtons.relabelButton(3, (char*) "+", true);
					maxChange = true;

				}
				//In minimum change mode - increase minimum temp
				else if ((minChange == true) && (maxChange == false)) {
					//Adjust temperatures from normal calibration
					currentVal = (int)calFunction(minTemp);
					//Check if minimum temp is still smaller than maximum temp
					if (currentVal < ((int)calFunction(maxTemp) - 1)) {
						while (((int)calFunction(minTemp)) < (currentVal + 1))
							minTemp = minTemp + 1;
					}
				}
			}
			//Prepare the preview image
			delay(10);
			createThermalImg(true);
			//Display the preview image
			display.drawBitmap(80, 40, 160, 120, image, 1);
		}
	}
}

/* Select the limits in Manual Mode*/
void limitChooser() {
	//Background & title
	liveMenuBackground();
	liveMenuTitle((char*) "Temp. Limits");
	//Draw the buttons
	touchButtons.deleteAllButtons();
	touchButtons.setTextFont(bigFont);
	touchButtons.addButton(15, 188, 140, 40, (char*) "Reset");
	touchButtons.addButton(165, 188, 140, 40, (char*) "OK");
	touchButtons.addButton(15, 48, 55, 120, (char*) "Min");
	touchButtons.addButton(250, 48, 55, 120, (char*) "Max");
	touchButtons.drawButtons();
	//Prepare the preview image
	delay(10);
	createThermalImg(true);
	//Display the preview image
	display.drawBitmap(80, 40, 160, 120, image, 1);
	//Draw the border for the preview image
	display.setColor(VGA_WHITE);
	display.drawRect(79, 39, 241, 161);
	//Go into the normal touch handler
	limitChooserHandler();
}

/* Temperature Limit Mode Selection */
bool tempLimits() {
	//Still in warmup, do not let the user do this
	if (calStatus == 0) {
		drawMessage((char*) "Please wait for sensor warmup!");
		delay(1500);
		return true;
	}
	//Title & Background
	liveMenuBackground();
	liveMenuTitle((char*)"Temp. Limits");
	//Draw the buttons
	touchButtons.deleteAllButtons();
	touchButtons.setTextFont(bigFont);
	touchButtons.addButton(15, 47, 140, 120, (char*) "Auto");
	touchButtons.addButton(165, 47, 140, 120, (char*) "Manual");
	touchButtons.addButton(15, 188, 140, 40, (char*) "Back");
	touchButtons.drawButtons();
	//Touch handler
	while (true) {
		//If touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//AUTO
			if (pressedButton == 0) {
				agcEnabled = true;
				return true;
			}
			//MANUAL
			else if (pressedButton == 1) {
				agcEnabled = false;
				limitChooser();
				return true;
			}
			//BACK
			else if (pressedButton == 2)
				return false;
		}
	}
}

/* Switch the current temperature menu item */
void liveMenuTempString(int pos) {
	char* text = (char*) "";
	switch (pos) {
	case 0:
		text = (char*) "Add point";
		break;
	case 1:
		text = (char*) "Rem. point";
		break;
	case 2:
		text = (char*) "Clear all";
		break;
	}
	liveMenuSelection(text);
}

/* Manu to select different options for temperature display */
bool tempMenu() {
	//Still in warmup, do not add points
	if (calStatus == 0) {
		drawMessage((char*) "Please wait for sensor warmup!");
		delay(1500);
		return true;
	}
	//Background
	liveMenuBackground();
	//Title
	liveMenuTitle((char*) "Add temp.");
	//Remove Exit button
	touchButtons.deleteButton(4);
	touchButtons.drawButtons();
	//Border
	display.setColor(255, 106, 0);
	display.drawRect(65, 57, 257, 111);
	//Draw the current item
	liveMenuTempString(colorScheme);
	//Save the current position inside the menu
	int pos = colorScheme;
	while (true) {
		//Touch screen pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//SELECT
			if (pressedButton == 3) {
				switch (pos) {
					//Add point
				case 0:
					tempPointFunction();
					break;
					//Remove point
				case 1:
					tempPointFunction(true);
					break;
					//Clear all
				case 2:
					clearTemperatures();
					break;
				}
				return true;
			}
			//BACK
			if (pressedButton == 2)
				return false;
			//BACKWARD
			else if (pressedButton == 0) {
				if (pos > 0)
					pos--;
				else if (pos == 0)
					pos = 2;
			}
			//FORWARD
			else if (pressedButton == 1) {
				if (pos < 2)
					pos++;
				else if (pos == 2)
					pos = 0;
			}
			//Change the menu name
			liveMenuTempString(pos);
		}
	}
}



/* Switch the current color scheme item */
void liveMenuColorString(int pos) {
	char* text = (char*) "";
	switch (pos) {
	case 0:
		text = (char*) "Arctic";
		break;
	case 1:
		text = (char*) "Black-Hot";
		break;
	case 2:
		text = (char*) "Blue-Red";
		break;
	case 3:
		text = (char*) "Coldest";
		break;
	case 4:
		text = (char*) "Contrast";
		break;
	case 5:
		text = (char*) "Double-Rain";
		break;
	case 6:
		text = (char*) "Gray-Red";
		break;
	case 7:
		text = (char*) "Glowbow";
		break;
	case 8:
		text = (char*) "Hottest";
		break;
	case 9:
		text = (char*) "Ironblack";
		break;
	case 10:
		text = (char*) "Lava";
		break;
	case 11:
		text = (char*) "Medical";
		break;
	case 12:
		text = (char*) "Rainbow";
		break;
	case 13:
		text = (char*) "Wheel 1";
		break;
	case 14:
		text = (char*) "Wheel 2";
		break;
	case 15:
		text = (char*) "Wheel 3";
		break;
	case 16:
		text = (char*) "White-Hot";
		break;
	case 17:
		text = (char*) "Yellow";
		break;
	}
	liveMenuSelection(text);
}

/* Choose the applied color scale */
bool changeColor() {
	//Save the current position inside the menu
	static byte changeColorPos = 0;
	//Background
	liveMenuBackground();
	//Title
	liveMenuTitle((char*) "Change Color");
	//Remove Exit button
	touchButtons.deleteButton(4);
	touchButtons.drawButtons();
	//Border
	display.setColor(255, 106, 0);
	display.drawRect(65, 57, 257, 111);
	//Draw the current item
	liveMenuColorString(colorScheme);
	while (true) {
		//Touch screen pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//SELECT
			if (pressedButton == 3) {
				changeColorScheme(&changeColorPos);
				return true;
			}
			//BACK
			if (pressedButton == 2)
				return false;
			//BACKWARD
			else if (pressedButton == 0) {
				if (changeColorPos > 0)
					changeColorPos--;
				else if (changeColorPos == 0)
					changeColorPos = 17;
			}
			//FORWARD
			else if (pressedButton == 1) {
				if (changeColorPos < 17)
					changeColorPos++;
				else if (changeColorPos == 17)
					changeColorPos = 0;
			}
			//Change the menu name
			liveMenuColorString(changeColorPos);
		}
	}
}

/* Switch the current display option item */
void liveMenuDisplayString(int pos) {
	char* text = (char*) "";
	switch (pos) {
		//Battery
	case 0:
		if (batteryEnabled)
			text = (char*) "Battery On";
		else
			text = (char*) "Battery Off";
		break;
		//Time
	case 1:
		if (timeEnabled)
			text = (char*) "Time On";
		else
			text = (char*) "Time Off";
		break;
		//Date
	case 2:
		if (dateEnabled)
			text = (char*) "Date On";
		else
			text = (char*) "Date Off";
		break;
		//Spot
	case 3:
		if (spotEnabled)
			text = (char*) "Spot On";
		else
			text = (char*) "Spot Off";
		break;
		//Colorbar
	case 4:
		if (colorbarEnabled)
			text = (char*) "Bar On";
		else
			text = (char*) "Bar Off";
		break;
		//Temperature Points
	case 5:
		if (pointsEnabled)
			text = (char*) "Points On";
		else
			text = (char*) "Points Off";
		break;
		//Storage
	case 6:
		if (storageEnabled)
			text = (char*) "Storage On";
		else
			text = (char*) "Storage Off";
		break;
		//Filter
	case 7:
		if (filterEnabled)
			text = (char*) "Filter On";
		else
			text = (char*) "Filter Off";
		break;
	}
	liveMenuSelection(text);
}

/* Change the display options */
bool displayOptions() {
	//Save the current position inside the menu
	static byte displayOptionsPos = 0;
	//Background
	liveMenuBackground();
	//Title
	liveMenuTitle((char*) "Display Options");
	//Remove Exit button
	touchButtons.deleteButton(4);
	touchButtons.drawButtons();
	//Rename OK button
	touchButtons.relabelButton(3, (char*) "Switch", true);
	//Border
	display.setColor(255, 106, 0);
	display.drawRect(65, 57, 257, 111);
	//Draw the current item
	liveMenuDisplayString(0);
	while (true) {
		//Touch screen pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//SELECT
			if (pressedButton == 3) {
				changeDisplayOptions(&displayOptionsPos);
			}
			//BACK
			if (pressedButton == 2) {
				touchButtons.relabelButton(3, (char*) "OK", true);
				return false;
			}
			//BACKWARD
			else if (pressedButton == 0) {
				if (displayOptionsPos > 0)
					displayOptionsPos--;
				else if (displayOptionsPos == 0)
					displayOptionsPos = 7;
			}
			//FORWARD
			else if (pressedButton == 1) {
				if (displayOptionsPos < 7)
					displayOptionsPos++;
				else if (displayOptionsPos == 7)
					displayOptionsPos = 0;
			}
			//Change the menu name
			liveMenuDisplayString(displayOptionsPos);
		}
	}
}

/* Select the action when the select button is pressed */
bool liveMenuSelect(byte* pos) {
	switch (*pos) {
		//Change Color
	case 0:
		return changeColor();
		break;
		//Temperature limits
	case 1:
		return tempLimits();
		break;
		//Calibration
	case 2:
		return calibrate();
		break;
		//Temp. points
	case 3:
		return tempMenu();
		break;
		//Display options
	case 4:
		return displayOptions();
		break;
		//Toggle Laser
	case 5:
		toggleLaser();
		break;
		//Display off
	case 6:
		digitalWrite(pin_lcd_backlight, LOW);
		//Wait for touch press
		while (!touch.touched());
		digitalWrite(pin_lcd_backlight, HIGH);
		return false;
		break;
	}
	return true;
}

/* Switch the current menu item */
void liveMenuMainString(byte* pos) {
	char* text = (char*) "";
	switch (*pos) {
		//Change Color
	case 0:
		text = (char*) "Chg. Color";
		break;
		//Temperature limits
	case 1:
		text = (char*) "Chg. Limits";
		break;
		//Calibration
	case 2:
		text = (char*) "Calibration";
		break;
		//Temp points
	case 3:
		text = (char*) "Set Points";
		break;
		//Display options
	case 4:
		text = (char*) "Disp. Opt.";
		break;
		//Laser On/Off
	case 5:
		if (laserEnabled)
			text = (char*) "Laser Off";
		else
			text = (char*) "Laser On";
		break;
		//Turn Display off
	case 6:
		text = (char*) "Display Off";
		break;
	}
	//Draws the current selection
	liveMenuSelection(text);
}

/* Draws the content of the live menu*/
void drawLiveMenu(byte* pos) {
	//Border
	display.setColor(VGA_BLACK);
	display.fillRoundRect(5, 5, 315, 235);
	//Background
	liveMenuBackground();
	//Buttons
	touchButtons.deleteAllButtons();
	touchButtons.setTextFont(bigFont);
	touchButtons.addButton(15, 45, 38, 77, (char*) "<", 0, true);
	touchButtons.addButton(267, 45, 38, 77, (char*) ">", 0, true);
	touchButtons.addButton(15, 188, 120, 40, (char*) "Back");
	touchButtons.addButton(100, 132, 130, 35, (char*) "OK");
	touchButtons.addButton(145, 188, 160, 40, (char*) "Main Menu");
	touchButtons.drawButtons();
	//Border
	display.setColor(255, 106, 0);
	display.drawRect(65, 57, 257, 111);
	//Title
	liveMenuTitle((char*) "Live Menu");
	//Current choice name
	liveMenuMainString(pos);
}

/* Touch Handler for the Live Menu */
bool liveMenuHandler(byte* pos) {
	//Main loop
	while (true) {
		//Touch screen pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//EXIT
			if (pressedButton == 4)
				return true;
			//SELECT
			if (pressedButton == 3) {
				//Leave menu
				if (liveMenuSelect(pos))
					break;
				else
					drawLiveMenu(pos);
			}
			//BACK
			else if (pressedButton == 2) {
				break;
			}
			//BACKWARD
			else if (pressedButton == 0) {
				if (*pos > 0)
					*pos = *pos - 1;
				else if (*pos == 0)
					*pos = 6;
			}
			//FORWARD
			else if (pressedButton == 1) {
				if (*pos < 6)
					*pos = *pos + 1;
				else if (*pos == 6)
					*pos = 0;
			}
			//Change the menu name
			liveMenuMainString(pos);
		}
	}
	return false;
}

/* Start live menu */
bool liveMenu() {
	//Live menu position
	static byte liveMenuPos = 0;
	bool rtn = 0;
	//Detach the interrupts
	detachInterrupts();
	//Draw content
	drawLiveMenu(&liveMenuPos);
	//Touch handler - return true if exit to Main menu, otherwise false
	rtn = liveMenuHandler(&liveMenuPos);
	//Restore old fonts
	display.setFont(smallFont);
	touchButtons.setTextFont(smallFont);
	//Delete the old buttons
	touchButtons.deleteAllButtons();
	//Re-attach the interrupts
	attachInterrupts();
	//Disable menu marker
	showMenu = false;
	//Return
	return rtn;
}