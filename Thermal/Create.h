/*
* Functions to create and display the thermal image
*/

/* A method to create a gaussian blur filter over an image */
void gaussianBlur() {
	const long numpixels = 19200;
	double lambda, dnu;
	float nu, boundaryscale, postscale;
	unsigned short *ptr;
	long i, x, y;
	float sigma = 1.5;
	lambda = (sigma * sigma) / 2.0;
	dnu = (1.0 + 2.0 * lambda - sqrt(1.0 + 4.0 * lambda)) / (2.0 * lambda);
	nu = (float)dnu;
	boundaryscale = (float)(1.0 / (1.0 - dnu));
	//For Lepton2 sensor or image save
	if ((imgSave == 3) || (leptonVersion != 1))
		postscale = (float)(pow(dnu / lambda, 2));
	//For Lepton3 sensor
	else
		postscale = (float)(pow(dnu / lambda, 1));
	//Filter horizontally along each row
	for (y = 0; y < 120; y++) {
		ptr = image + (160 * y);
		ptr[0] *= boundaryscale;
		//Filter rightwards
		for (x = 1; x < 160; x++)
			ptr[x] += nu * ptr[x - 1];
		//For Lepton2 sensor or image save
		if ((imgSave == 3) || (leptonVersion != 1)) {
			ptr[x = 159] *= boundaryscale;
			//Filter leftwards
			for (; x > 0; x--)
				ptr[x - 1] += nu * ptr[x];
		}
	}
	//Filter vertically along each column
	for (x = 0; x < 160; x++) {
		ptr = image + x;
		ptr[0] *= boundaryscale;
		//Filter downwards
		for (i = 160; i < numpixels; i += 160)
			ptr[i] += nu * ptr[i - 160];
		//For Lepton2 sensor or image save
		if ((imgSave == 3) || (leptonVersion != 1)) {
			ptr[i = numpixels - 160] *= boundaryscale;
			//Filter upwards
			for (; i > 0; i -= 160)
				ptr[i - 160] += nu * ptr[i];
		}
	}
	for (i = 0; i < numpixels; i++) {
		image[i] *= postscale;
		if (image[i] > 255)
			image[i] = 255;
	}
	return;
}

bool savePackage(byte line, byte segment = 0, bool save = false) {
	//Go through the video pixels for one video line
	for (int column = 0; column < 80; column++) {
		uint16_t result = (uint16_t)(leptonFrame[2 * column + 4] << 8
			| leptonFrame[2 * column + 5]);
		if (result == 0) {
			return false;
		}
		//Lepton2 Rotated (ThermocamV4)
		if ((mlx90614Version == 0) && (leptonVersion != 1)) {
			//For saving raw data, use small array
			if (save) {
				if (!rotationEnabled)
					rawValues[column + (line * 80)] = result;
				else
					rawValues[4799 - (column + (line * 80))] = result;
			}
			else {
				if (!rotationEnabled) {
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
		}
		//Lepton2 Non-Rotated
		else if ((mlx90614Version == 1) && (leptonVersion != 1)) {
			//For saving raw data, use small array
			if (save) {
				if (!rotationEnabled)
					rawValues[4799 - (column + (line * 80))] = result;
				else
					rawValues[column + (line * 80)] = result;
			}
			else {
				if (!rotationEnabled) {
					image[19199 - ((line * 2 * 160) + (column * 2))] = result;
					image[19199 - ((line * 2 * 160) + (column * 2) + 1)] = result;
					image[19199 - ((line * 2 * 160) + 160 + (column * 2))] = result;
					image[19199 - ((line * 2 * 160) + 160 + (column * 2) + 1)] = result;
				}
				else {
					image[(line * 2 * 160) + (column * 2)] = result;
					image[(line * 2 * 160) + (column * 2) + 1] = result;
					image[(line * 2 * 160) + 160 + (column * 2)] = result;
					image[(line * 2 * 160) + 160 + (column * 2) + 1] = result;
				}
			}
		}
		//Lepton3
		else if (leptonVersion == 1) {
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
void refreshTempPoints(bool save) {
	//Lepton2
	if (leptonVersion != 1) {
		for (int y = 0; y < 60; y = y + 5) {
			for (int x = 0; x < 80; x = x + 5) {
				if (showTemp[(x / 5) + (16 * (y / 5))] != 0) {
					if (save)
						showTemp[(x / 5) + (16 * (y / 5))] = rawValues[x + (y * 160)];
					else
						showTemp[(x / 5) + (16 * (y / 5))] = image[(x * 2) + (y * 2 * 160)];
				}
			}
		}
	}
	//Lepton3
	else {
		for (int y = 0; y < 120; y = y + 10) {
			for (int x = 0; x < 160; x = x + 10) {
				if (showTemp[(x / 10) + (16 * (y / 10))] != 0) {
					showTemp[(x / 10) + (16 * (y / 10))] = image[x + (y * 160)];
				}
			}
		}
	}
}

/* Get one image from the Lepton module */
void getTemperatures(bool save) {
	byte leptonError, segmentNumbers, line;
	//For Lepton2 sensor, get only one segment per frame
	if (leptonVersion != 1)
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
					if (!savePackage(line, segment, save)) {
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

/* Scale the values from 0 - 255 */
void scaleValues(bool small) {
	//Calculate the scale
	float scale = (colorElements - 1.0) / (maxTemp - minTemp);
	//Go through rawValues array
	if (small) {
		for (int i = 0; i < 4800; i++) {
			//Limit values
			if (rawValues[i] > maxTemp)
				rawValues[i] = maxTemp;
			if (rawValues[i] < minTemp)
				rawValues[i] = minTemp;
			rawValues[i] = (rawValues[i] - minTemp) * scale;
		}
	}
	//Go through image array
	else {
		for (int i = 0; i < 19200; i++) {
			//Limit values
			if (image[i] > maxTemp)
				image[i] = maxTemp;
			if (image[i] < minTemp)
				image[i] = minTemp;
			image[i] = (image[i] - minTemp) * scale;
		}
	}
}

/* Go through the array of temperatures and find min and max temp */
void limitValues(bool small) {
	maxTemp = 0;
	minTemp = 65535;
	uint16_t temp;
	//Go through small rawValues array
	if (small) {
		for (int i = 0; i < 4800; i++) {
			//Get value
			temp = rawValues[i];
			//Find maximum temp
			if (temp > maxTemp)
				maxTemp = temp;
			//Find minimum temp
			if (temp < minTemp)
				minTemp = temp;
		}
	}
	//Go through big image array
	else {
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
}

/* Convert the lepton values to RGB colors */
void convertColors(bool small) {
	//Go through the rawValues array
	if (small) {
		for (int i = 0; i < 4800; i++) {
			uint16_t value = rawValues[i];
			//Look for the corresponding RGB values
			uint8_t red = colorMap[3 * value];
			uint8_t green = colorMap[3 * value + 1];
			uint8_t blue = colorMap[3 * value + 2];
			//Convert to RGB565
			rawValues[i] = (((red & 248) | green >> 5) << 8) | ((green & 28) << 3 | blue >> 3);
		}
	}
	//Go through the image array
	else {
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

}

/* Creates a thermal image and stores it in the array */
void createThermalImg(bool menu) {
	//Receive the temperatures over SPI
	getTemperatures();
	//Refresh the temp points if required
	if (pointsEnabled)
		refreshTempPoints();
	//Find min and max if not in manual mode and limits not locked
	if ((agcEnabled) && (!limitsLocked)) {
		//Limit values if we are in the menu or not in cold/hot mode
		if (menu || ((colorScheme != 3) && (colorScheme != 8)))
			limitValues();
	}
	//Scale the values
	scaleValues();
	//Apply filter if enabled, in menu or when saving the image to bitmap
	if ((filterEnabled) || menu || ((imgSave == 3) && convertEnabled))
		gaussianBlur();
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
	//Show thermal image
	displayThermalImg();
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

/* Create and display the thermal image on screen */
void displayThermalImg() {
	//If the image save marker was set
	if (imgSave == 2) {
		//Show message on screen
		showMsg((char*) "Save Thermal..");
		//Set marker to create image
		imgSave = 3;
	}
	//Create the thermal image
	createThermalImg();
	//Draw thermal image on screen if created previously
	if (imgSave != 2) display.writeScreen(image);
	//If the image has been created, set to save
	if (imgSave == 3)
		imgSave = 1;
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
				display.drawPixel(xpos, ypos);
				display.drawPixel(xpos + 1, ypos);
				display.drawPixel(xpos, ypos + 1);
				display.drawPixel(xpos + 1, ypos + 1);
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