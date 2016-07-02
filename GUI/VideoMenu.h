/*
* Video Menu
*/

/* Variables */
//Video save interval in seconds
int16_t videoInterval;

/* Methods */

/* Switch the video interval string*/
void videoIntervalString(int pos) {
	char* text = (char*) "";
	switch (pos) {
		//1 second
	case 0:
		text = (char*) "1 second";
		break;
		//5 seconds
	case 1:
		text = (char*) "5 seconds";
		break;
		//10 seconds
	case 2:
		text = (char*) "10 seconds";
		break;
		//20 seconds
	case 3:
		text = (char*) "20 seconds";
		break;
		//30 seconds
	case 4:
		text = (char*) "30 seconds";
		break;
		//1 minute
	case 5:
		text = (char*) "1 minute";
		break;
		//5 minutes
	case 6:
		text = (char*) "5 Minute";
		break;
		//10 minutes
	case 7:
		text = (char*) "10 Minutes";
		break;
	}
	//Draws the current selection
	liveMenuSelection(text);
}

/* Touch Handler for the video interval chooser */
bool videoIntervalHandler() {
	//Save the current position inside the menu
	byte pos = 0;
	//Main loop
	while (true) {
		//Touch screen pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//SELECT
			if (pressedButton == 3) {
				switch (pos) {
					//1 second
				case 0:
					videoInterval = 1;
					break;
					//5 seconds
				case 1:
					videoInterval = 5;
					break;
					//10 seconds
				case 2:
					videoInterval = 10;
					break;
					//20 seconds
				case 3:
					videoInterval = 20;
					break;
					//30 seconds
				case 4:
					videoInterval = 30;
					break;
					//1 minute
				case 5:
					videoInterval = 60;
					break;
					//5 minutes
				case 6:
					videoInterval = 300;
					break;
					//10 minutes
				case 7:
					videoInterval = 600;
					break;
				}
				return true;
			}
			//BACK
			else if (pressedButton == 2) {
				return false;
			}
			//BACKWARD
			else if (pressedButton == 0) {
				if (pos > 0)
					pos--;
				else if (pos == 0)
					pos = 7;
			}
			//FORWARD
			else if (pressedButton == 1) {
				if (pos < 7)
					pos++;
				else if (pos == 7)
					pos = 0;
			}
			//Change the menu name
			videoIntervalString(pos);
		}
	}
}

/* Start video menu to choose interval */
bool videoIntervalChooser() {
	bool rtn = 0;
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
	touchButtons.addButton(15, 188, 140, 40, (char*) "Back");
	touchButtons.addButton(100, 132, 130, 35, (char*) "OK");
	touchButtons.drawButtons();
	//Border
	display.setColor(255, 106, 0);
	display.drawRect(65, 57, 257, 111);
	//Title
	liveMenuTitle((char*) "Choose interval");
	//Current choice name
	videoIntervalString(0);
	//Touch handler - return true if exit to Main menu, otherwise false
	rtn = videoIntervalHandler();
	//Restore old fonts
	display.setFont(smallFont);
	touchButtons.setTextFont(smallFont);
	//Delete the old buttons
	touchButtons.deleteAllButtons();
	return rtn;
}

/* Display the video capture screen contents */
void refreshCapture() {
	//Fill image array for Lepton2 sensor
	if (leptonVersion != leptonVersion_3_Shutter)
		fillImageArray();
	//Filter image - not in normal mode
	if (videoInterval != 0) {
		if (filterType == filterType_box)
			boxFilter();
		else if (filterType == filterType_gaussian)
			gaussianFilter();
	}
	//Scale values
	scaleValues();
	//Convert lepton data to RGB565 colors
	convertColors();
	//Draw thermal image on screen
	display.writeScreen(image);
	//Display additional infos
	displayInfos();
}

/* Captures video frames in an interval */
void videoCaptureInterval(int16_t* remainingTime, uint16_t* framesCaptured, char* dirname) {
	//Measure time it takes to display everything
	long measure = millis();
	char buffer[30];
	if ((*remainingTime == 0) || (*framesCaptured == 0)) {
		//Show save message
		if (*framesCaptured != 0)
			showMsg((char*)"Saving frame..");
		//Send capture command to camera if activated and there is enough time
		if ((visualEnabled) && (videoInterval >= 10))
			captureVisualImage();
		//Save video raw frame
		saveRawData(false, dirname, *framesCaptured);
		refreshCapture();
		//Save visual image if activated
		if ((visualEnabled) && (videoInterval >= 10)) {
			//Display message
			display.setFont(bigFont);
			display.print((char*) "Interval capture", CENTER, 20);
			showMsg((char*) "Save Visual..");
			//Create filename to save data
			char filename[] = "00000.JPG";
			frameFilename(filename, *framesCaptured);
			//Create file
			createJPGFile(filename, dirname);
			//Save visual image
			saveVisualImage();
			//Reset counter
			int16_t elapsed = (millis() - measure) / 1000;
			*remainingTime = videoInterval - elapsed;
		}
		else
			*remainingTime = videoInterval;
		//Raise capture counter
		*framesCaptured = *framesCaptured + 1;
	}
	else {
		//Refresh display content
		refreshCapture();
		//Display title
		display.setFont(bigFont);
		display.print((char*) "Interval capture", CENTER, 20);
		//Display remaining time message
		sprintf(buffer, "Saving next frame in %d second(s)", *remainingTime);
		display.setFont(smallFont);
		display.print(buffer, CENTER, 210);
	}
	//Wait rest of the time
	measure = millis() - measure;
	if (measure < 1000)
		delay(1000 - measure);
	//Decrease remaining time by one
	*remainingTime -= 1;
}

/* Captures video frames normal */
void videoCaptureNormal(uint16_t* framesCaptured, char* dirname) {
	char buffer[30];
	//Save video raw frame
	saveRawData(false, dirname, *framesCaptured);
	//Raise capture counter
	*framesCaptured = *framesCaptured + 1;
	//Refresh display content
	refreshCapture();
	//Display title
	display.setFont(bigFont);
	display.print((char*) "Recording..", CENTER, 20);
	//Display status update
	sprintf(buffer, "Frames captured: %-5d", *framesCaptured);
	display.setFont(smallFont);
	display.print(buffer, CENTER, 210);
}

/* This screen is shown during the video capture */
void videoCapture() {
	//Help variables
	char dirname[20];
	int16_t delayTime = videoInterval;
	uint16_t framesCaptured = 0;
	//Allocate space for raw values
	rawValues = (unsigned short*)calloc(4800, sizeof(unsigned short));
	//Create folder 
	createVideoFolder(dirname);
	//Update display
	drawMessage((char*) "Please wait..");
	//Attach the interrupts
	attachInterrupts();
	//Set text colors
	display.setColor(VGA_WHITE);
	display.setBackColor(VGA_TRANSPARENT);
	//Main loop
	while (videoSave) {
		//Touch - turn display on or off
		if (!digitalRead(pin_touch_irq)) {
			digitalWrite(pin_lcd_backlight, !(checkScreenLight()));
			while (!digitalRead(pin_touch_irq));
		}
		//Normal  mode
		if (videoInterval == 0)
			videoCaptureNormal(&framesCaptured, dirname);
		//Time interval save
		else {
			videoCaptureInterval(&delayTime, &framesCaptured, dirname);
		}
		//Maximum amount of frames reached or disk space out, return
		if ((framesCaptured == 65535) || (getSDSpace() < 1000)) {
			drawMessage((char*) "Not enough space, abort!");
			break;
		}
	}
	//Turn the display on if it was off before
	if (!checkScreenLight())
		enableScreenLight();
	//Post processing for interval videos if enabled
	if ((framesCaptured > 0)  && (videoInterval != 0)) {
		videoSave = true;
		//Ask the user if he wants to convert that video
		if (convertPrompt(true)) {
			drawMessage((char*) "Capture finished ! Converting..");
			delay(1000);
			//Attach the Button interrupt
			attachInterrupt(pin_button, buttonIRQ, RISING);
			proccessVideoFrames(framesCaptured, dirname);
		}
	}
	else {
		drawMessage((char*) "Video capture finished !");
		delay(1000);
	}
	//Deallocate space
	free(rawValues);
	//Re-attach the interrupts
	attachInterrupts();
	//Disable mode
	videoSave = false;
	imgSave = false;
}

/* Video mode chooser, normal or interval */
bool videoModeChooser() {
	//For old HW, check if the SD card is there
	if (mlx90614Version == mlx90614Version_old) {
		if (!checkSDCard()) {
			//Re-attach the interrupts
			attachInterrupts();
			//Disable mode
			videoSave = false;
			imgSave = false;
			return false;
		}
	}
	//Check if there is at least 1MB of space left
	if (getSDSpace() < 1000) {
		showMsg((char*) "Int. space full!");
		//Re-attach the interrupts
		attachInterrupts();
		//Disable mode
		videoSave = false;
		imgSave = false;
		return false;
	}
	//Title & Background
	liveMenuBackground();
	liveMenuTitle((char*)"Video mode");
	//Draw the buttons
	touchButtons.deleteAllButtons();
	touchButtons.setTextFont(bigFont);
	touchButtons.addButton(15, 47, 140, 120, (char*) "Normal");
	touchButtons.addButton(165, 47, 140, 120, (char*) "Interval");
	touchButtons.addButton(15, 188, 140, 40, (char*) "Back");
	touchButtons.drawButtons();
	//Touch handler
	while (true) {
		//If touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//Normal
			if (pressedButton == 0) {
				videoInterval = 0;
				break;
			}
			//Interval
			else if (pressedButton == 1) {
				if (!videoIntervalChooser())
					return false;
				break;
			}
			//BACK
			else if (pressedButton == 2) {
				//Re-attach the interrupts
				attachInterrupts();
				//Disable mode
				videoSave = false;
				imgSave = false;
				return false;
			}
		}
	}
	return true;
}