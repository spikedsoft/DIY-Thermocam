/*
* Video Menu
*/

/* Switch the video interval setting */
void liveMenuVideoString(int pos) {
	char* text = (char*) "";
	switch (pos) {
		//Fast
	case 0:
		text = (char*) "Fast";
		break;
		//Normal
	case 1:
		text = (char*) "Normal";
		break;
		//1 Second
	case 2:
		text = (char*) "1 Second";
		break;
		//5 Seconds
	case 3:
		text = (char*) "5 Seconds";
		break;
		//10 Seconds
	case 4:
		text = (char*) "10 Seconds";
		break;
		//30 Seconds
	case 5:
		text = (char*) "30 Seconds";
		break;
		//1 Minute
	case 6:
		text = (char*) "1 Minute";
		break;
		//5 Minutes
	case 7:
		text = (char*) "5 Minutes";
		break;
		//10 Minutes
	case 8:
		text = (char*) "10 Minutes";
		break;
	}
	//Draws the current selection
	liveMenuSelection(text);
}

/* Touch Handler for the Video Menu */
bool videoMenuHandler() {
	//Save the current position inside the menu
	int pos = 0;
	//Main loop
	while (true) {
		//Touch screen pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//SELECT
			if (pressedButton == 3) {
				switch (pos) {
					//Fast
				case 0:
					videoInterval = -1;
					break;
					//Normal
				case 1:
					videoInterval = 0;
					break;
					//1 Second
				case 2:
					videoInterval = 1;
					break;
					//5 Seconds
				case 3:
					videoInterval = 5;
					break;
					//10 Seconds
				case 4:
					videoInterval = 10;
					break;
					//30 Seconds
				case 5:
					videoInterval = 30;
					break;
					//1 Minute
				case 6:
					videoInterval = 60;
					break;
					//5 Minutes
				case 7:
					videoInterval = 300;
					break;
					//10 Minutes
				case 8:
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
					pos = 8;
			}
			//FORWARD
			else if (pressedButton == 1) {
				if (pos < 8)
					pos++;
				else if (pos == 8)
					pos = 0;
			}
			//Change the menu name
			liveMenuVideoString(pos);
		}
	}
}

/* Display the video capture screen contents */
void refreshCapture() {
	//Fill image array for Lepton2 sensor
	if(leptonVersion == 0)
		fillImageArray();
	//Scale values
	scaleValues();
	//Filter image - not in fast or normal mode and only for Lepton2 sensor
	if (videoInterval > 0)
		gaussianBlur(image, 160, 120, 1.5f, 1);
	//Convert lepton data to RGB565 colors
	convertColors();
	//Draw thermal image on screen
	display.writeScreen(image);
	//Show the spot in the middle
	if (spotEnabled)
		showSpot(true);
	//Show the color bar
	if (calibrationDone)
		showColorBar();
	//Display title
	display.setFont(bigFont);
	display.setBackColor(VGA_TRANSPARENT);
	display.print((char*) "Recording..", CENTER, 20);
	display.setFont(smallFont);
	display.setBackColor(VGA_WHITE);
	display.setColor(VGA_BLACK);
}

/* Captures video frames in an interval */
void videoCaptureInterval(int* remainingTime, uint16_t* framesCaptured, char* dirname) {
	//Measure time it takes to display everything
	long measure = millis();
	char buffer[30];
	if ((*remainingTime == 0) || (*framesCaptured == 0)) {
		//Send capture command to camera if activated
		if (videosType == 1)
			captureVisualImage();
		//Save video raw frame
		saveRawData(false, dirname, *framesCaptured);
		refreshCapture();
		//Save visual image if activated
		if (videosType == 1)
			saveVisualFrame(*framesCaptured, dirname);
		//Raise capture counter
		*framesCaptured = *framesCaptured + 1;
		//Reset counter
		*remainingTime = videoInterval;
	}
	//Display remaining time message
	sprintf(buffer, "Next frame in %d second(s)", *remainingTime);
	display.print(buffer, CENTER, 210);
	//Wait rest of the time
	measure = millis() - measure;
	if (measure < 1000)
		delay(1000 - measure);
	//Decrease remaining time by one
	*remainingTime -= 1;
}

/* Captures video frames live */
void videoCaptureLive(uint16_t* framesCaptured, char* dirname) {
	long measure = millis();
	char buffer[30];
	//Send capture command to camera if activated in normal mode
	if ((videosType == 1) && (videoInterval == 0))
		captureVisualImage();
	//Save video raw frame
	saveRawData(false, dirname, *framesCaptured);
	//Raise capture counter
	*framesCaptured = *framesCaptured + 1;
	//Refresh display content
	if ((((*framesCaptured % 10) == 0) && (videoInterval == -1)) ||
		(((*framesCaptured % 4) == 0) && (videoInterval == 0)) || (*framesCaptured == 1))
		refreshCapture();
	//Display status update
	sprintf(buffer, "Frames captured: %d", *framesCaptured);
	display.print(buffer, CENTER, 210);
	//Save visual image if activated
	if ((videosType == 1) && (videoInterval == 0))
		//Do not use this in fast mode
		saveVisualFrame(*framesCaptured - 1, dirname);
	//Wait some time
	//Fast mode 
	if (videoInterval == -1) {
		measure = millis() - measure;
		if (measure < 100)
			delay(100 - measure);
	}
	//Normal mode
	else if (videoInterval == 0) {
		measure = millis() - measure;
		if (measure < 250)
			delay(250 - measure);
	}
}

/* This screen is shown during the video capture */
void videoCapture() {
	//Help variables
	bool live = false;
	char dirname[20];
	int remainingTime;
	uint16_t framesCaptured = 0;
	//Allocate space for raw values
	rawValues = (unsigned short*)calloc(4800, sizeof(unsigned short));
	//Use live mode when interval is zero or -1
	if ((videoInterval == 0) || (videoInterval == - 1))
		live = true;
	//Set remaining time value
	else
		remainingTime = videoInterval;
	//Create folder 
	createVideoFolder(dirname);
	//Update display
	drawMessage((char*) "Please wait..");
	//Attach the interrupts
	attachInterrupts();
	//Main loop
	while (videoSave) {
		//Normal or fast mode
		if (live) {
			videoCaptureLive(&framesCaptured, dirname);
		}
		//Time interval save
		else {
			videoCaptureInterval(&remainingTime, &framesCaptured, dirname);
		}
		//Maximum amount of frames reached or disk space out, return
		if ((framesCaptured == 65535) || (getSDSpace() < 1000)) {
			drawMessage((char*) "Not enough space, abort!");
			break;
		}
	}
	//Post processing if enabled
	if ((framesCaptured > 0) && (videosFormat == 1)) {
		drawMessage((char*) "Capture finished ! Converting..");
		delay(1000);
		videoSave = true;
		//Attach the Button interrupt
		attachInterrupt(pin_button, buttonIRQ, RISING);
		proccessVideoFrames(framesCaptured, dirname);
	}
	else {
		drawMessage((char*) "Video capture finished !");
		delay(1000);
	}
	//Deallocate space
	free(rawValues);
	//Turn the display on if it was off before
	bool displayState = digitalRead(pin_lcd_backlight);
	if (displayState == false)
		displayOn(true);
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
	liveMenuTitle((char*) "Video - Interval");
	//Current choice name
	liveMenuVideoString(0);
	//Touch handler - return true if exit to Main menu, otherwise false
	rtn = videoMenuHandler();
	//Restore old fonts
	display.setFont(smallFont);
	touchButtons.setTextFont(smallFont);
	//Delete the old buttons
	touchButtons.deleteAllButtons();
	return rtn;
}