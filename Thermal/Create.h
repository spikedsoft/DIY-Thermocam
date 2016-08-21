/*
* Functions to create and display the thermal image
*/

/* Filter the image with a box blur filter (LP) */
void boxFilter() {
	int sum;

	for (int y = 1; y < 119; y++) {
		for (int x = 1; x < 159; x++) {
			sum = 0;
			for (int k = -1; k <= 1; k++) {
				for (int j = -1; j <= 1; j++) {
					sum += image[(y - j) * 160 + (x - k)];
				}
			}
			image[(y * 160) + x] = (unsigned short)(sum / 9.0);
		}
	}
}

/* Filter the image with a gaussian blur filter (LP) */
void gaussianFilter() {
	byte gaussianKernel[3][3] = {
		{ 1, 2, 1 },
		{ 2, 4, 2 },
		{ 1, 2, 1 }
	};
	int sum;

	for (int y = 1; y < 119; y++) {
		for (int x = 1; x < 159; x++) {
			sum = 0;
			for (int k = -1; k <= 1; k++) {
				for (int j = -1; j <= 1; j++) {
					sum += gaussianKernel[j + 1][k + 1] * image[(y - j) * 160 + (x - k)];
				}
			}
			image[(y * 160) + x] = (unsigned short)(sum / 16.0);
		}
	}
}

/* Store one package of 80 columns into RAM */
bool savePackage(byte line, byte segment = 0) {
	//Go through the video pixels for one video line
	for (int column = 0; column < 80; column++) {
		uint16_t result = (uint16_t)(leptonFrame[2 * column + 4] << 8
			| leptonFrame[2 * column + 5]);
		if (result == 0) {
			return false;
		}
		//Lepton2
		if (leptonVersion != leptonVersion_3_Shutter) {
			if (((mlx90614Version == mlx90614Version_old) && (rotationEnabled == false)) ||
				((mlx90614Version == mlx90614Version_new) && (rotationEnabled == true))) {
				image[(line * 2 * 160) + (column * 2)] = result;
				image[(line * 2 * 160) + (column * 2) + 1] = result;
				image[(line * 2 * 160) + 160 + (column * 2)] = result;
				image[(line * 2 * 160) + 160 + (column * 2) + 1] = result;
			}
			else {
				image[19199 - ((line * 2 * 160) + (column * 2))] = result;
				image[19199 - ((line * 2 * 160) + (column * 2) + 1)] = result;
				image[19199 - ((line * 2 * 160) + 160 + (column * 2))] = result;
				image[19199 - ((line * 2 * 160) + 160 + (column * 2) + 1)] = result;
			}
		}
		//Lepton3
		else {
			//Fill array non rotated
			if (!rotationEnabled) {
				switch (segment) {
				case 1:
					image[19199 - (((line / 2) * 160) + ((line % 2) * 80) + (column))] = result;
					break;
				case 2:
					image[14399 - (((line / 2) * 160) + ((line % 2) * 80) + (column))] = result;
					break;
				case 3:
					image[9599 - (((line / 2) * 160) + ((line % 2) * 80) + (column))] = result;
					break;
				case 4:
					image[4799 - (((line / 2) * 160) + ((line % 2) * 80) + (column))] = result;
					break;
				}
			}
			//Fill array rotated
			else {
				switch (segment) {
				case 1:
					image[((line / 2) * 160) + ((line % 2) * 80) + (column)] = result;
					break;
				case 2:
					image[4800 + (((line / 2) * 160) + ((line % 2) * 80) + (column))] = result;
					break;
				case 3:
					image[9600 + (((line / 2) * 160) + ((line % 2) * 80) + (column))] = result;
					break;
				case 4:
					image[14400 + (((line / 2) * 160) + ((line % 2) * 80) + (column))] = result;
					break;
				}
			}
		}
	}
	return true;
}

/* Refresh the temperature points*/
void refreshTempPoints() {
	for (int y = 0; y < 12; y++) {
		for (int x = 0; x < 16; x++) {
			if (showTemp[(x + (16 * y))] != 0) {
				showTemp[(x + (16 * y))] = image[(x * 10) + (y * 10 * 160) + 805];
			}
		}
	}
}

/* Get one image from the Lepton module */
void getTemperatures() {
	byte leptonError, segmentNumbers, line;
	//For Lepton2 sensor, get only one segment per frame
	if (leptonVersion != leptonVersion_3_Shutter)
		segmentNumbers = 1;
	//For Lepton3 sensor, get four packages per frame
	else
		segmentNumbers = 4;
	//Begin SPI transmission
	leptonBeginSPI();
	for (byte segment = 1; segment <= segmentNumbers; segment++) {
		leptonError = 0;
		do {
			for (line = 0; line < 60; line++) {
				//If line matches expectation
				if (leptonReadFrame(line, segment)) {
					if (!savePackage(line, segment)) {
						//Stabilize framerate
						delayMicroseconds(800);
						//Raise lepton error
						leptonError++;
						break;
					}
				}
				//Reset if the expected line does not match the answer
				else {
					if (leptonError == 255) {
						//If show menu was entered
						if (showMenu) {
							leptonEndSPI();
							return;
						}
						//Reset segment
						segment = 1;
						//Reset lepton error
						leptonError = 0;
						//End Lepton SPI
						leptonEndSPI();
						//Short delay
						delay(186);
						//Begin Lepton SPI
						leptonBeginSPI();
						break;
					}
					else {
						//Stabilize framerate
						delayMicroseconds(800);
						//Raise lepton error
						leptonError++;
						break;
					}
				}
			}
		} while (line != 60);
	}
	//End Lepton SPI
	leptonEndSPI();
}

void findMinMaxPositions()
{
	unsigned short min, max;
	min = USHRT_MAX;
	max = 0;

	for (int i = 0; i < 19200; i++)
	{
		if (image[i] < min)
		{
			image_pos_mintemp = i;
			min = image[i];
		}
		if (image[i] > max)
		{
			image_pos_maxtemp = i;
			max = image[i];
		}
	}
}

/* Scale the values from 0 - 255 */
void scaleValues() {
	//Calculate the scale
	float scale = (colorElements - 1.0) / (maxTemp - minTemp);
	//Go through image array
	for (int i = 0; i < 19200; i++) {
		//Limit values
		if (image[i] > maxTemp)
			image[i] = maxTemp;
		if (image[i] < minTemp)
			image[i] = minTemp;
		image[i] = (image[i] - minTemp) * scale;
	}
}

/* Go through the array of temperatures and find min and max temp */
void limitValues() {
	maxTemp = 0;
	minTemp = 65535;
	uint16_t temp;
	for (int i = 0; i < 19200; i++) {
		//Get value
		temp = image[i];
		//Find maximum temp
		if (temp > maxTemp)
			maxTemp = temp;
		//Find minimum temp
		if (temp < minTemp)
			minTemp = temp;
	}
}

/* Convert the lepton values to RGB colors */
void convertColors() {
	for (int i = 0; i < 19200; i++) {
		uint16_t value = image[i];
		//Look for the corresponding RGB values
		uint8_t red = colorMap[3 * value];
		uint8_t green = colorMap[3 * value + 1];
		uint8_t blue = colorMap[3 * value + 2];
		//Convert to RGB565
		image[i] = (((red & 248) | green >> 5) << 8) | ((green & 28) << 3 | blue >> 3);
	}
}

/* Create the visual or combined image display */
void createVisCombImg() {
	//Send capture command
	captureVisualImage();
	//Receive the temperatures over SPI
	getTemperatures();
	//Compensate calibration with object temp
	compensateCalib();
	//Refresh the temp points if required
	if (pointsEnabled)
		refreshTempPoints();
	//Find min and max if not in manual mode and limits not locked
	if ((agcEnabled) && (!limitsLocked)) {
		//Limit values if we are in the menu or not in cold/hot mode
		if ((colorScheme != colorScheme_coldest) && (colorScheme != colorScheme_hottest))
			limitValues();
	}
	//Find the minimum and maximum pixels in the image
	findMinMaxPositions();
	//Scale the values
	scaleValues();
	//Convert lepton data to RGB565 colors
	convertColors();
	//Get the visual image and decompress it combined
	getVisualImage();
}

/* Creates a thermal image and stores it in the array */
void createThermalImg(bool menu) {
	//Receive the temperatures over SPI
	getTemperatures();
	//Compensate calibration with object temp
	compensateCalib();

	//Refresh the temp points if required
	if (pointsEnabled)
		refreshTempPoints();

	//Find min and max if not in manual mode and limits not locked
	if ((agcEnabled) && (!limitsLocked)) {
		//Limit values if we are in the menu or not in cold/hot mode
		if (menu || ((colorScheme != colorScheme_coldest) && (colorScheme != colorScheme_hottest)))
			limitValues();
	}

	//If image save, save the raw data
	if (imgSave == imgSave_create)
		saveRawData(true, saveFilename);

	//Apply low-pass filter
	if (filterType == filterType_box)
		boxFilter();
	else if (filterType == filterType_gaussian)
		gaussianFilter();

	//Find the minimum and maximum pixels in the image
	findMinMaxPositions();
	//Scale the values
	scaleValues();
	//Convert lepton data to RGB565 colors
	convertColors();
}

/* Read the x and y coordinates when touch screen is pressed for Add Point */
void getTouchPos(int* x, int* y) {
	int iter = 0;
	TS_Point point;
	unsigned long tx = 0;
	unsigned long ty = 0;
	//Wait for touch press
	while (!touch.touched());
	//While touch pressed, iterate over readings
	while (touch.touched() == true) {
		point = touch.getPoint();
		if ((point.x >= 0) && (point.x <= 320) && (point.y >= 0)
			&& (point.y <= 240)) {
			tx += point.x;
			ty += point.y;
			iter++;
		}
	}
	*x = tx / iter;
	*y = ty / iter;
}

/* Function to add or remove a measurement point */
void tempPointFunction(bool remove) {
	int x, y;

	//Create thermal image
	if (displayMode == displayMode_thermal)
		createThermalImg();
	//Create visual or combined image
	else
		createVisCombImg();
	//Show it on the screen
	showImage();

	//Set text color, font and background
	display.setColor(VGA_WHITE);
	display.setBackColor(VGA_TRANSPARENT);
	display.setFont(smallFont);
	//Show current temperature points
	showTemperatures();
	//Get touched coordinates
	getTouchPos(&x, &y);
	//Divide through 20 to match array size
	x /= 20;
	y /= 20;
	if (x < 0)
		x = 0;
	if (x > 15)
		x = 15;
	if (y < 0)
		y = 0;
	if (y > 11)
		y = 11;
	//Remove point from screen by setting it to zero
	if (remove) {
	outerloop:
		for (int i = x - 1; i <= x + 1; i++) {
			for (int j = y - 1; j <= y + 1; j++) {
				if (((j * 16 + i) >= 0) && ((j * 16 + i) < 192)) {
					if (showTemp[(j * 16) + i] != 0) {
						showTemp[(j * 16) + i] = 0;
						goto outerloop;
					}
				}
			}
		}
	}
	//Mark point for refreshing in the next image
	else
		showTemp[(y * 16) + x] = 1;
}

/* Clears the show temperatures array */
void clearTemperatures() {
	//Set all showTemps to zero
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 12; y++) {
			showTemp[(y * 16) + x] = 0;
		}
	}
}

/* Shows the temperatures over the image on the screen */
void showTemperatures() {
	int xpos, ypos;
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 12; y++) {
			if (showTemp[(y * 16) + x] != 0) {
				xpos = x * 20;
				ypos = y * 20;
				if (ypos <= 12)
					ypos = 13;
				display.print((char*) ".", xpos, ypos - 10);
				xpos -= 22;
				if (xpos < 0)
					xpos = 0;
				ypos += 6;
				if (ypos > 239)
					ypos = 239;
				display.printNumF(calFunction(showTemp[(y * 16) + x]), 2, xpos, ypos);
			}
		}
	}
}

static void _displayMinMaxPoint(int pixelIndex, const char *str)
{
	int x, y;
	x = (pixelIndex % 160) * 2;
	y = (pixelIndex / 160) * 2;

	if (y > 240)
		y = 240;

	word savedColor = display.getColor();
	word pixelColor = display.readPixel(x<3 ? x + 3 : x - 3, y);
	unsigned int pixelColorSum = ((pixelColor >> 8) & 0xF8) + ((pixelColor >> 3) & 0xFC) + (pixelColor << 3);
	unsigned int pixelColorAvg = pixelColorSum / 3;
	if (pixelColorAvg > 0xf8)
		display.setColor(0);
	else
		display.setColor(0xff, 0xff, 0xff);

	display.drawLine(x / 2, y / 2, x / 2, y / 2);

	x += 4;
	if (x >= 310)
		x -= 10;
	if (y > 230)
		y = 230;

	display.print(str, x, y);
	display.setColor(savedColor);
}

void showMinPoint()
{
	_displayMinMaxPoint(image_pos_mintemp, (const char *)"C");
}

void showMaxPoint()
{
	_displayMinMaxPoint(image_pos_maxtemp, (const char *)"H");
}