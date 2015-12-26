/*
* Save images and videos to the internal storage
*/

/* Creates a filename from the current time & date */
void createSDName(char* filename, bool folder = false) {
	char buffer[5];
	//Year
	itoa(year(), buffer, 10);
	strncpy(&filename[0], buffer, 4);
	//Month
	itoa(month(), buffer, 10);
	if (month() < 10) {
		filename[4] = '0';
		strncpy(&filename[5], buffer, 1);
	}
	else {
		strncpy(&filename[4], buffer, 2);
	}
	//Day
	itoa(day(), buffer, 10);
	if (day() < 10) {
		filename[6] = '0';
		strncpy(&filename[7], buffer, 1);
	}
	else {
		strncpy(&filename[6], buffer, 2);
	}
	//Hour
	itoa(hour(), buffer, 10);
	if (hour() < 10) {
		filename[8] = '0';
		strncpy(&filename[9], buffer, 1);
	}
	else {
		strncpy(&filename[8], buffer, 2);
	}
	//Minute
	itoa(minute(), buffer, 10);
	if (minute() < 10) {
		filename[10] = '0';
		strncpy(&filename[11], buffer, 1);
	}
	else {
		strncpy(&filename[10], buffer, 2);
	}
	//Second
	itoa(second(), buffer, 10);
	if (second() < 10) {
		filename[12] = '0';
		if (!folder)
			strncpy(&filename[13], buffer, 1);
		else
			strcpy(&filename[13], buffer);
	}
	else {
		if (!folder)
			strncpy(&filename[12], buffer, 2);
		else
			strcpy(&filename[12], buffer);
	}
}

/* Shows on the screen that we saved an image */
void showMsg(char* msg, bool bottom = false) {
	//Set Text Color
	setColor();
	//set Background transparent
	display.setBackColor(VGA_TRANSPARENT);
	//Give the user a hint that it tries to save
	display.setFont(bigFont);
	if (bottom)
		display.print(msg, CENTER, 160);
	else
		display.print(msg, CENTER, 80);
	display.setFont(smallFont);
}

/* Creates a bmp file for the thermal image */
void createBMPFile(char* filename) {
	//File extension and open
	strcpy(&filename[14], ".BMP");
	sdFile.open(filename, O_RDWR | O_CREAT | O_AT_END);
	//Create the Thermal Image File header
	const char bmp_header[66] = { 0x42, 0x4D, 0x36, 0x60, 0x09, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
		0x80, 0x02, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0x01, 0x00, 0x10,
		0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x60, 0x09, 0x00, 0xC4, 0x0E,
		0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00,
		0x1F, 0x00, 0x00, 0x00 };
	//Write the BMP header
	for (int i = 0; i < 66; i++) {
		char ch = bmp_header[i];
		sdFile.write((uint8_t*)&ch, 1);
	}
}

/* Creates a jpg file for the visual image */
void createJPGFile(char* filename) {
	//Begin SD Transmission
	startAltClockline(true);
	//File extension and open
	strcpy(&filename[14], ".JPG");
	sdFile.open(filename, O_RDWR | O_CREAT | O_AT_END);
	//Switch clockline
	endAltClockline();
}

/* Creates a folder for the video capture */
void createVideoFolder(char* dirname) {
	startAltClockline(true);
	//Build the dir from the current time & date
	createSDName(dirname, true);
	//Create folder
	sd.mkdir(dirname);
	//Go into that folder
	sd.chdir(dirname);
	endAltClockline();
}

/* Saves a thermal image to the sd card */
void saveThermalImage(char* filename) {
	//Begin SD Transmission
	startAltClockline(true);
	//Create file
	createBMPFile(filename);
	//Allocate space for sd buffer
	uint8_t* sdBuffer = (uint8_t*)calloc(640, sizeof(uint8_t));
	unsigned short pixel;
	//Save 320*60 pixels from the screen at one time
	for (int i = 3; i >= 0; i--) {
		endAltClockline();
		//Read pixels from the display
		display.readScreen(i, image);
		startAltClockline();
		for (byte y = 0; y < 60; y++) {
			//Write them into the sd buffer
			for (uint16_t x = 0; x < 320; x++) {
				pixel = image[((59 - y) * 320) + x];
				sdBuffer[x * 2] = pixel & 0x00FF;
				sdBuffer[(x * 2) + 1] = (pixel & 0xFF00) >> 8;
			}
			//Write them to the sd card and double for better resolution
			for (int i = 0; i < 2; i++) {
				for (uint16_t x = 0; x < 320; x++) {
					sdFile.write(sdBuffer[x * 2]);
					sdFile.write(sdBuffer[(x * 2) + 1]);
					sdFile.write(sdBuffer[x * 2]);
					sdFile.write(sdBuffer[(x * 2) + 1]);
				}
			}
		}
	}
	//De-allocate space
	free(sdBuffer);
	//Close file
	sdFile.close();
	//End SD Transmission
	endAltClockline();
}

/* Saves images to the internal storage */
void saveImage() {
	//Wake camera up if needed and take image
	if (imagesType == 1)
		captureVisualImage();
	//Build filename from the current time & date
	char filename[20];
	createSDName(filename);
	//Allocate space for raw values
	rawValues = (unsigned short*)calloc(4800, sizeof(unsigned short));
	//Save Raw Values
	saveRawData(true, filename);
	//Deallocate space again
	free(rawValues);
	//Save Bitmap image if activated
	if (imagesFormat == 1)
		saveThermalImage(filename);
	//Eventually save optical image
	if (imagesType == 1) {
		//Deallocate space
		free(image);
		//Create file
		createJPGFile(filename);
		//Display message
		showMsg((char*) "Save Visual..");
		//Save visual image
		saveVisualImage();
		//Allocate space
		image = (unsigned short*)calloc(19200, sizeof(unsigned short));
	}
	//Show Message on screen
	if (imagesType == 1)
		showMsg((char*) "All saved!", true);
	else
		showMsg((char*) "Thermal saved!");
	delay(500);
}

/* Converts a float to four bytes */
void floatToBytes(uint8_t* farray, float val)
{
	unsigned long d = *(unsigned long *)&val;
	farray[0] = d & 0x00FF;
	farray[1] = (d & 0xFF00) >> 8;
	farray[2] = (d & 0xFF0000) >> 16;
	farray[3] = (d & 0xFF000000) >> 24;
}

/* Converts four bytes back to float */
float bytesToFloat(uint8_t* farray)
{
	unsigned long d;
	d = (farray[3] << 24) | (farray[2] << 16)
		| (farray[1] << 8) | (farray[0]);
	float val = *(float *)&d;
	return val;
}

/* Creates the filename for the video frames */
void frameFilename(char* filename, uint16_t count) {
	filename[0] = '0' + count / 10000 % 10;
	filename[1] = '0' + count / 1000 % 10;
	filename[2] = '0' + count / 100 % 10;
	filename[3] = '0' + count / 10 % 10;
	filename[4] = '0' + count % 10;
}

/* Save visual image inside the video capture */
void saveVisualFrame(uint16_t count, char* dirname) {
	//Create filename to save data
	char filename[] = "00000.JPG";
	frameFilename(filename, count);
	//Switch Clock to Alternative
	startAltClockline();
	//Go into the video folder
	sd.chdir(dirname);
	// Open the file for writing
	sdFile.open(filename, O_RDWR | O_CREAT | O_AT_END);
	//Switch clockline
	endAltClockline();
	//Deallocate space
	free(image);
	free(rawValues);
	//Save visual image
	saveVisualImage();
	//Allocate space
	rawValues = (unsigned short*)calloc(4800, sizeof(unsigned short));
	image = (unsigned short*)calloc(19200, sizeof(unsigned short));
}

/* Read video frame from file for processing*/
void readVideoFrame(uint16_t count, char* dirname) {
	byte msb, lsb;
	uint16_t valueCount;
	unsigned short* valueArray;
	//For the Lepton2 sensor, use 4800 raw values
	if (leptonVersion == 0) {
		valueCount = 4800;
		valueArray = rawValues;
	}
	//For the Lepton3 sensor, use 19200 raw values
	else {
		valueCount = 19200;
		valueArray = image;
	}
	//Create filename to open saved data
	char filename[] = "00000.DAT";
	frameFilename(filename, count);
	//Switch Clock to Alternative
	startAltClockline();
	//Go into the video folder
	sd.chdir(dirname);
	// Open the file for reading
	sdFile.open(filename, O_READ);
	//Read all lepton raw values from file
	for (int i = 0; i < valueCount; i++) {
		msb = sdFile.read();
		lsb = sdFile.read();
		valueArray[i] = (((msb) << 8) + lsb);
	}
	//Read Min
	msb = sdFile.read();
	lsb = sdFile.read();
	minTemp = (((msb) << 8) + lsb);
	//Read Max
	msb = sdFile.read();
	lsb = sdFile.read();
	maxTemp = (((msb) << 8) + lsb);
	//Read object temperature if enabled
	if (spotEnabled) {
		uint8_t farray[4];
		for (int i = 0; i < 4; i++) {
			farray[i] = sdFile.read();
		}
		mlx90614Temp = bytesToFloat(farray);
	}
	//Close data file
	sdFile.close();
}

/* Save video frame to image file */
void saveVideoFrame(uint16_t count, char* dirname) {
	//Create filename to save data
	char filename[] = "00000.BMP";
	frameFilename(filename, count);
	//Switch Clock to Alternative
	startAltClockline();
	//Go into the video folder
	sd.chdir(dirname);
	// Open the file for writing
	sdFile.open(filename, O_RDWR | O_CREAT | O_AT_END);
	//Header for frame content
	const char bmp_header[66] = { 0x42, 0x4D, 0x36, 0x58, 0x02, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00,
		0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x10, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x58, 0x02,
		0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00,
		0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00 };
	//Write the BMP header
	for (int i = 0; i < 66; i++) {
		char ch = bmp_header[i];
		sdFile.write((uint8_t*)&ch, 1);
	}
	//Help variables for saving
	unsigned short pixel;
	//Allocate space for sd buffer
	uint8_t* sdBuffer = (uint8_t*)calloc(640, sizeof(uint8_t));
	//Save 320*60 pixels from the screen at one time
	for (int i = 3; i >= 0; i--) {
		endAltClockline();
		//Read pixels from the display
		display.readScreen(i, image);
		startAltClockline();
		for (byte y = 0; y < 60; y++) {
			//Write them into the sd buffer
			for (uint16_t x = 0; x < 320; x++) {
				pixel = image[((59 - y) * 320) + x];
				sdFile.write(pixel & 0x00FF);
				sdFile.write((pixel & 0xFF00) >> 8);
			}
		}
	}
	//De-allocate space
	free(sdBuffer);
	//Close the file
	sdFile.close();
	//Switch Clock back to Standard
	endAltClockline();
}

/* Fills the image array with the raw values */
void fillImageArray() {
	//Fill image array
	for (int y = 0; y < 60; y++) {
		for (int x = 0; x < 80; x++) {
			image[(y * 2 * 160) + (x * 2)] = rawValues[(y * 80) + x];
			image[(y * 2 * 160) + (x * 2) + 1] = rawValues[(y * 80) + x];
			image[(y * 2 * 160) + 160 + (x * 2)] = rawValues[(y * 80) + x];
			image[(y * 2 * 160) + 160 + (x * 2) + 1] = rawValues[(y * 80) + x];
		}
	}
}

/* Proccess video frames */
void proccessVideoFrames(uint16_t framesCaptured, char* dirname) {
	char buffer[14];
	display.setBackColor(VGA_TRANSPARENT);
	for (uint16_t count = 0; count < framesCaptured; count++) {
		//Check if there is at least 1MB of space left
		if (getSDSpace() < 1000) {
			drawMessage((char*) "No space, stop conversion..");
			delay(1000);
			return;
		}
		//Check if the user wants to abort conversion
		if (videoSave == false) {
			drawMessage((char*) "Video proccessing aborted!");
			delay(1000);
			return;
		}
		//Read frame
		readVideoFrame(count, dirname);
		//Fill image array for Lepton2 sensor
		if (leptonVersion == 0)
			fillImageArray();
		//Scale values
		scaleValues();
		//Apply gaussian filter
		gaussianBlur(image, 160, 120, 1.5f, 1);
		//Convert lepton data to RGB565 colors
		convertColors();
		//Draw thermal image on screen
		endAltClockline();
		display.writeScreen(image);
		//Show the spot in the middle
		if (spotEnabled)
			showSpot(true);
		//Show the color bar
		if (calibrationDone)
			showColorBar();
		//Show the image number
		sprintf(buffer, "%5d / %-5d", count + 1, framesCaptured);
		setColor();
		display.print(buffer, CENTER, 225);
		//Save frame to image file
		saveVideoFrame(count, dirname);
	}
	//All images converted!
	drawMessage((char*) "Video proccessing finished !");
	delay(1000);
}

/* Saves raw data for an image or an video frame */
void saveRawData(bool isImage, char* name, uint16_t framesCaptured) {
	uint16_t valueCount;
	unsigned short* valueArray;
	//For the Lepton2 sensor, use 4800 raw values
	if (leptonVersion == 0) {
		valueCount = 4800;
		valueArray = rawValues;
	}
	//For the Lepton3 sensor, use 19200 raw values
	else {
		valueCount = 19200;
		valueArray = image;
	}
	//Get temperatures
	getTemperatures(true);
	//Find min and max
	if (agcEnabled) {
		maxTemp = 0;
		minTemp = 65535;
		uint16_t temp;
		for (int i = 0; i < valueCount; i++) {
			temp = valueArray[i];
			//Find maximum temp
			if (temp > maxTemp)
				maxTemp = temp;
			//Find minimum temp
			if (temp < minTemp)
				minTemp = temp;
		}
	}
	//Start SD
	startAltClockline(true);
	//Create filename for image
	if (isImage) {
		strcpy(&name[14], ".DAT");
		sdFile.open(name, O_RDWR | O_CREAT | O_AT_END);
	}
	//Create filename for video frame
	else {
		char filename[] = "00000.DAT";
		frameFilename(filename, framesCaptured);
		sd.chdir(name);
		sdFile.open(filename, O_RDWR | O_CREAT | O_AT_END);
	}
	//Write the lepton raw values
	for (int i = 0; i < valueCount; i++) {
		sdFile.write((valueArray[i] & 0xFF00) >> 8);
		sdFile.write(valueArray[i] & 0x00FF);
	}
	//Write min and max
	sdFile.write((minTemp & 0xFF00) >> 8);
	sdFile.write(minTemp & 0x00FF);
	sdFile.write((maxTemp & 0xFF00) >> 8);
	sdFile.write(maxTemp & 0x00FF);
	//Write the object temp 
	uint8_t farray[4];
	//Get object temp
	mlx90614GetTemp();
	//Write object temp
	floatToBytes(farray, mlx90614Temp);
	for (int i = 0; i < 4; i++)
		sdFile.write(farray[i]);
	//Write the color scheme
	sdFile.write(colorScheme);
	//Write the temperature format
	sdFile.write(tempFormat);
	//Write the show spot attribute
	sdFile.write(spotEnabled);
	//Write the calibration done attribute
	sdFile.write(calibrationDone);
	//Write Calibration slope
	floatToBytes(farray, (float)calSlope);
	for (int i = 0; i < 4; i++)
		sdFile.write(farray[i]);
	//Write Calibration offset
	floatToBytes(farray, (float)calOffset);
	for (int i = 0; i < 4; i++)
		sdFile.write(farray[i]);
	//Close the file
	sdFile.close();
	//Switch Clock back to Standard
	endAltClockline();
}

/* Save a screenshot to the sd Card */
/*void saveScreenshot() {
	if (Serial.available() > 0) {
		Serial.read();
		Serial.println("Saving Screenshot..");
		//Switch Clock to Alternative
		startAltClockline(true);
		//Choose a filename
		char filename[] = "SCREENSHOT000.BMP";
		for (int i = 0; i < 1000; i++) {
			filename[10] = '0' + i / 100 % 10;
			filename[11] = '0' + i / 10 % 10;
			filename[12] = '0' + i % 10;
			if (!sd.exists(filename))
				break;
		}
		sdFile.open(filename, O_RDWR | O_CREAT | O_AT_END);
		//Create the Thermal Image File header
		const char bmp_header[66] = { 0x42, 0x4D, 0x36, 0x60, 0x09, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
			0x80, 0x02, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0x01, 0x00, 0x10,
			0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x60, 0x09, 0x00, 0xC4, 0x0E,
			0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00,
			0x1F, 0x00, 0x00, 0x00 };
		//Write the BMP header
		for (int i = 0; i < 66; i++) {
			char ch = bmp_header[i];
			sdFile.write((uint8_t*)&ch, 1);
		}
		//Allocate space for sd buffer
		uint8_t* sdBuffer = (uint8_t*)calloc(640, sizeof(uint8_t));
		unsigned short pixel;
		//Save 320*60 pixels from the screen at one time
		for (int i = 3; i >= 0; i--) {
			endAltClockline();
			//Read pixels from the display
			display.readScreen(i, image);
			startAltClockline();
			for (byte y = 0; y < 60; y++) {
				//Write them into the sd buffer
				for (uint16_t x = 0; x < 320; x++) {
					pixel = image[((59 - y) * 320) + x];
					sdBuffer[x * 2] = pixel & 0x00FF;
					sdBuffer[(x * 2) + 1] = (pixel & 0xFF00) >> 8;
				}
				//Write them to the sd card and double for better resolution
				for (int i = 0; i < 2; i++) {
					for (uint16_t x = 0; x < 320; x++) {
						sdFile.write(sdBuffer[x * 2]);
						sdFile.write(sdBuffer[(x * 2) + 1]);
						sdFile.write(sdBuffer[x * 2]);
						sdFile.write(sdBuffer[(x * 2) + 1]);
					}
				}
			}
		}
		//De-allocate space
		free(sdBuffer);
		//Close file
		sdFile.close();
		//End SD Transmission
		endAltClockline();
		Serial.println("Screenshot saved !");
	}
}*/