/*
* Menu to load images and videos from the internal storage
*/

/* Display the image on the screen */
void displayRawData() {
	//Select Color Scheme
	selectColorScheme();
	//Fill image array for Lepton2 sensor
	if (leptonVersion != 1)
		fillImageArray();
	//Scale values
	scaleValues();
	//Apply gaussian filter
	gaussianBlur();
	//Convert lepton data to RGB565 colors
	convertColors();
	//Display on screen
	display.writeScreen(image);
	//Display additional information
	displayInfos();
}

void displayGUI(int imgCount, char* infoText) {
	//Set text color
	display.setColor(VGA_WHITE);
	//set Background transparent
	display.setBackColor(VGA_TRANSPARENT);
	display.setFont(bigFont);
	//Exit to main menu
	display.print((char*) "Exit", 250, 10);
	//Find image by time and date
	display.print((char*) "Find", 5, 10);
	//Display prev/next if there is more than one image
	if (imgCount != 1) {
		display.print((char*) "<", 10, 110);
		display.print((char*) ">", 295, 110);
	}
	//Convert image to bitmap
	display.print((char*) "Convert", 5, 210);
	//Delete image from internal 
	display.print((char*) "Delete", 220, 210);
	display.setFont(smallFont);
	//Display either frame number or image date and time
	display.print(infoText, CENTER, 12);
}

/* Asks the user if he wants to delete the video */
void deleteVideo(char* dirname) {
	//Title & Background
	drawTitle((char*) "Delete Video");
	display.setColor(VGA_WHITE);
	display.setFont(smallFont);
	display.setBackColor(127, 127, 127);
	display.print((char*)"Do you want to delete this video ?", CENTER, 66);
	display.print((char*)"This will also remove the", CENTER, 105);
	display.print((char*)"other related files to it. ", CENTER, 125);
	//Draw the buttons
	touchButtons.deleteAllButtons();
	touchButtons.setTextFont(bigFont);
	touchButtons.addButton(15, 160, 140, 55, (char*) "No");
	touchButtons.addButton(165, 160, 140, 55, (char*) "Yes");
	touchButtons.drawButtons();
	touchButtons.setTextFont(smallFont);
	updateInfos(true);
	//Touch handler
	while (true) {
		updateInfos(false);
		//If touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//YES
			if (pressedButton == 1) {
				drawMessage((char*) "Delete video..");
				//Start SD
				startAltClockline(true);
				//Go into the video folder
				sd.chdir("/");
				sd.chdir(dirname);
				//Delete all files
				uint16_t videoCounter = 0;
				bool exists;
				char filename[] = "00000.DAT";
				//Go through the frames
				while (1) {
					//Get the frame name
					frameFilename(filename, videoCounter);
					//Check frame existance
					exists = sd.exists(filename);
					//If the frame does not exists, end remove
					if (!exists)
						break;
					//Otherwise remove file
					else
						sd.remove(filename);
					//Remove Bitmap if there
					strcpy(&filename[5], ".BMP");
					if (sd.exists(filename))
						sd.remove(filename);
					//Remove Jpeg if there
					strcpy(&filename[5], ".JPG");
					if (sd.exists(filename))
						sd.remove(filename);
					//Reset ending
					strcpy(&filename[5], ".DAT");
					//Raise counter
					videoCounter++;
				}
				//Switch back to the root
				sd.chdir("/");
				//Remove the folder itself
				sd.rmdir(dirname);
				//End SD
				endAltClockline();
				drawMessage((char*) "Video deleted!");
				delay(1000);
				return;
			}
			//NO
			else if (pressedButton == 0) {
				return;
			}
		}
	}
}

/* Asks the user if he wants to delete the image */
void deleteImage(char* filename) {
	//Title & Background
	drawTitle((char*) "Delete Image");
	display.setColor(VGA_WHITE);
	display.setFont(smallFont);
	display.setBackColor(127, 127, 127);
	display.print((char*)"Do you want to delete this image ?", CENTER, 66);
	display.print((char*)"This will also remove the", CENTER, 105);
	display.print((char*)"other related files to it. ", CENTER, 125);
	//Draw the buttons
	touchButtons.deleteAllButtons();
	touchButtons.setTextFont(bigFont);
	touchButtons.addButton(15, 160, 140, 55, (char*) "No");
	touchButtons.addButton(165, 160, 140, 55, (char*) "Yes");
	touchButtons.drawButtons();
	touchButtons.setTextFont(smallFont);
	updateInfos(true);
	//Touch handler
	while (true) {
		updateInfos(false);
		//If touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//YES
			if (pressedButton == 1) {
				drawMessage((char*) "Delete image..");
				//Start SD
				startAltClockline();
				//Delete .DAT file
				sd.remove(filename);
				//Delete .JPG file
				strcpy(&filename[14], ".JPG");
				if ((visualEnabled == 1) && (sd.exists(filename)))
					sd.remove(filename);
				//Delete .BMP file
				strcpy(&filename[14], ".BMP");
				if ((convertEnabled == 1) && (sd.exists(filename)))
					sd.remove(filename);
				endAltClockline();
				drawMessage((char*) "Image deleted!");
				delay(1000);
				return;
			}
			//NO
			else if (pressedButton == 0) {
				return;
			}
		}
	}
}

/* Asks the user if he really wants to convert the image/video */
bool convertPrompt(bool infosHidden = false) {
	//Title & Background
	drawTitle((char*) "Conversion Prompt");
	display.setColor(VGA_WHITE);
	display.setFont(smallFont);
	display.setBackColor(127, 127, 127);
	display.print((char*)"Do you want to convert ?", CENTER, 66);
	display.print((char*)"That proccess will create", CENTER, 105);
	display.print((char*)"bitmap(s) out of the raw data.", CENTER, 125);
	//Draw the buttons
	touchButtons.deleteAllButtons();
	touchButtons.setTextFont(bigFont);
	touchButtons.addButton(15, 160, 140, 55, (char*) "Yes");
	touchButtons.addButton(165, 160, 140, 55, (char*) "No");
	touchButtons.drawButtons();
	touchButtons.setTextFont(smallFont);
	//Wait for touch release
	while (touch.touched());
	if (!infosHidden)
		updateInfos(true);
	//Touch handler
	while (true) {
		if (!infosHidden)
			updateInfos(false);
		//If touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//YES
			if (pressedButton == 0) {
				return true;
			}
			//NO
			else if (pressedButton == 1) {
				return false;
			}
		}
	}
}

/* Convert a raw image lately to BMP */
void convertImage(char* filename) {
	strcpy(&filename[14], ".BMP");
	startAltClockline(true);
	bool exists = sd.exists(filename);
	endAltClockline();
	//If image is already converted, return
	if (exists) {
		drawMessage((char*) "Image is already converted!");
		delay(500);
		strcpy(&filename[14], ".DAT");
		return;
	}
	//If the user does not want to convert, return
	if (!convertPrompt()) {
		strcpy(&filename[14], ".DAT");
		return;
	}
	//Convert
	drawMessage((char*) "Converting image to BMP..");
	delay(500);
	//Display on screen
	display.writeScreen(image);
	//Show additional information
	displayInfos();
	//Save image
	saveThermalImage(filename);
	drawMessage((char*) "Image converted !");
	delay(500);
	strcpy(&filename[14], ".DAT");
}

/* Convert a raw video lately to BMP frames */
void convertVideo(char* dirname) {
	uint16_t frames = getVideoFrameNumber(dirname);
	char filename[] = "00000.BMP";
	//Switch Clock to Alternative
	startAltClockline(true);
	//Go into the folder
	sd.chdir(dirname);
	//Get the frame name of the first frame
	frameFilename(filename, 0);
	bool exists = sd.exists(filename);
	endAltClockline();
	//If video is already converted, return
	if (exists) {
		drawMessage((char*) "Video is already converted!");
		delay(500);
		return;
	}
	//If the user does not want to convert the video, return
	if (!convertPrompt())
		return;
	//Convert
	drawMessage((char*) "Converting video to BMP..");
	delay(500);
	videoSave = true;
	attachInterrupt(pin_button, buttonIRQ, RISING);
	proccessVideoFrames(frames, dirname);
	detachInterrupt(pin_button);
	videoSave = false;
}

/* Touch handler for the load image/video menu */
bool loadTouchHandler(bool isimg, char* filename, byte* choice, int imgCount, char* dirname = NULL) {
	if (touch.touched() == true) {
		TS_Point p = touch.getPoint();
		uint16_t x = p.x;
		uint16_t y = p.y;
		//Find
		if ((x >= 15) && (x <= 80) && (y >= 15) && (y <= 60)) {
			*choice = 1;
			return true;
		}
		//Delete
		else if ((x >= 200) && (x <= 305) && (y >= 180) && (y <= 225)) {
			//Image
			if (isimg) {
				deleteImage(filename);
				*choice = 2;
				return true;
			}
			//Video
			else {
				deleteVideo(dirname);
				*choice = 2;
				return true;
			}
		}
		//Previous image
		else if ((x >= 0) && (x <= 40) && (y >= 100) && (y <= 140) && (imgCount != 1)) {
			*choice = 3;
			return true;
		}
		//Next image
		else if ((x >= 280) && (x <= 319) && (y >= 100) && (y <= 140) && (imgCount != 1)) {
			*choice = 4;
			return true;
		}
		//Exit
		else if ((x >= 240) && (x <= 305) && (y >= 15) && (y <= 60)) {
			*choice = 5;
			return true;
		}
		//Convert
		else if ((x >= 15) && (x <= 140) && (y >= 180) && (y <= 225)) {
			//Image
			if (isimg) {
				convertImage(filename);
				return true;
			}
			//Video
			else {
				convertVideo(dirname);
				return true;
			}
		}
	}
	return false;
}

/* Shows a menu where the user can choose the time & date items for the image */
int loadMenu(char* title, int* array, int length) {
	//Draw the title on screen
	drawTitle(title);
	//Draw the Buttons
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 70, 70, (char*) "-");
	touchButtons.addButton(230, 60, 70, 70, (char*) "+");
	touchButtons.addButton(20, 150, 130, 70, (char*) "Back");
	touchButtons.addButton(170, 150, 130, 70, (char*) "Choose");
	touchButtons.drawButtons();
	int currentPos = 0;
	//Display the first element for the array
	drawCenterElement(array[currentPos]);
	updateInfos(true);
	while (1) {
		//Update the additional information
		updateInfos(false);
		//Touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Minus
			if (pressedButton == 1) {
				//Decrease element by one
				if (currentPos > 0)
					currentPos--;
				//Go from lowest to highest element
				else if (currentPos == 0)
					currentPos = length - 1;
				//Draw it on screen
				drawCenterElement(array[currentPos]);
			}
			//Plus
			else if (pressedButton == 0) {
				//Increase element by one
				if (currentPos < (length - 1))
					currentPos++;
				//Go from highest to lowest element
				else if (currentPos == (length - 1))
					currentPos = 0;
				//Draw it on screen
				drawCenterElement(array[currentPos]);
			}
			//Back - return minus 1
			else if (pressedButton == 2) {
				return -1;
			}
			//Set - return element's position
			else if (pressedButton == 3) {
				return currentPos;
			}
		}
	}
}