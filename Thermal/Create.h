/*
* Functions to create and display the thermal image
*/

/* A method to create a gaussian blur filter over an image */
void gaussianBlur(unsigned short *img, long width, long height, float sigma,
	int numsteps) {
	const long numpixels = width * height;
	double lambda, dnu;
	float nu, boundaryscale, postscale;
	unsigned short *ptr;
	long i, x, y;
	int step;
	lambda = (sigma * sigma) / (2.0 * numsteps);
	dnu = (1.0 + 2.0 * lambda - sqrt(1.0 + 4.0 * lambda)) / (2.0 * lambda);
	nu = (float)dnu;
	boundaryscale = (float)(1.0 / (1.0 - dnu));
	postscale = (float)(pow(dnu / lambda, 2 * numsteps));
	//Filter horizontally along each row
	for (y = 0; y < height; y++) {
		for (step = 0; step < numsteps; step++) {
			ptr = img + width * y;
			ptr[0] *= boundaryscale;
			//Filter rightwards
			for (x = 1; x < width; x++)
				ptr[x] += nu * ptr[x - 1];
			ptr[x = width - 1] *= boundaryscale;
			//Filter leftwards
			for (; x > 0; x--)
				ptr[x - 1] += nu * ptr[x];
		}
	}
	//Filter vertically along each column
	for (x = 0; x < width; x++) {
		for (step = 0; step < numsteps; step++) {
			ptr = img + x;
			ptr[0] *= boundaryscale;
			//Filter downwards
			for (i = width; i < numpixels; i += width)
				ptr[i] += nu * ptr[i - width];
			ptr[i = numpixels - width] *= boundaryscale;
			//Filter upwards
			for (; i > 0; i -= width)
				ptr[i - width] += nu * ptr[i];
		}
	}
	for (i = 0; i < numpixels; i++) {
		img[i] *= postscale;
		if (img[i] > 255)
			img[i] = 255;
	}
	return;
}

/* Get one image from the Lepton module */
void getTemperatures(bool save = false) {
	byte leptonError = 0;
	byte line, segmentNumbers;
	//For Early-Bird #1 and #2, get only one segment per frame
	if (leptonVersion == 0)
		segmentNumbers = 1;
	//For Lepton3 sensor, get four packages per frame
	else
		segmentNumbers = 4;
	//Begin SPI Transmission
	beginLeptonSPI();
	//Get number of packages per frame according to the lepton revision
	for (int segment = 1; segment <= segmentNumbers; segment++) {
		do {
			leptonError = 0;
			for (line = 0; line < 60; line++) {
				//Reset if the expected line does not match the answer
				if (!leptonReadFrame(line, segment)) {
					//Reset line to -1, will be zero in the next cycle
					line = -1;
					//Raise Error count
					leptonError++;
					//Little delay
					delay(1);
					//If the Error count is too high, reset the device
					if (leptonError > 100) {
						//Segment Error
						segment = 0;
						//Re-Sync the Lepton
						endLeptonSPI();
						delay(186);
						beginLeptonSPI();
						break;
					}
				}
				//If line matches answer, save the packet
				else {
					//Go through the video pixels for one video line
					for (int column = 0; column < 80; column++) {
						uint16_t result = (uint16_t)(leptonFrame[2 * column + 4] << 8
							| leptonFrame[2 * column + 5]);
						//Early-Bird #1
						if ((mlx90614Version == 0) && (leptonVersion == 0)){
							//For saving raw data, use small array
							if (save) {
								rawValues[column + (line * 80)] = result;
							}
							else {
								image[(line * 2 * 160) + (column * 2)] = result;
								image[(line * 2 * 160) + (column * 2) + 1] = result;
								image[(line * 2 * 160) + 160 + (column * 2)] = result;
								image[(line * 2 * 160) + 160 + (column * 2) + 1] = result;
							}
						}
						//Early-Bird #2
						else if ((mlx90614Version == 1) && (leptonVersion == 0)) {
							//For saving raw data, use small array
							if (save) {
								rawValues[4799 - (column + (line * 80))] = result;
							}
							else {
								image[19199 - ((line * 2 * 160) + (column * 2))] = result;
								image[19199 - ((line * 2 * 160) + (column * 2) + 1)] = result;
								image[19199 - ((line * 2 * 160) + 160 + (column * 2))] = result;
								image[19199 - ((line * 2 * 160) + 160 + (column * 2) + 1)] = result;
							}
						}
						//Batch #1
						else if (leptonVersion == 1) {
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
					}
				}
			}
		} while ((leptonError > 100) || (line != 60));
	}
	//End Lepton SPI
	endLeptonSPI();
}

/* Scale the values from 0 - 255 */
void scaleValues() {
	//Calculate the scale
	float scale = 255.0 / (maxTemp - minTemp);
	for (int i = 0; i < 19200; i++)
		image[i] = (image[i] - minTemp) * scale;
}

/* Go through the array of temperatures and find min and max temp */
void limitValues() {
	maxTemp = 0;
	minTemp = 65535;
	uint16_t temp;
	for (int i = 0; i < 19200; i++) {
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
	//Go through the array
	for (int i = 0; i < 19200; i++) {
		uint16_t value = image[i];
		//Hot
		if (colorScheme == 3) {
			if (value < (255 - grayscaleLevel))
				colormap = colormap_grayscale;
			else
				colormap = colormap_rainbow;
		}
		//Cold
		if (colorScheme == 4) {
			if (value > grayscaleLevel)
				colormap = colormap_grayscale;
			else
				colormap = colormap_rainbow;
		}
		//Look for the corresponding RGB values
		uint8_t red = colormap[3 * value];
		uint8_t green = colormap[3 * value + 1];
		uint8_t blue = colormap[3 * value + 2];
		//Convert to RGB565
		image[i] = (((red & 248) | green >> 5) << 8) | ((green & 28) << 3 | blue >> 3);
	}
}

/* Show the current object temperature on screen*/
void showSpot(bool save) {
	//Set text color
	setColor();
	display.setBackColor(VGA_TRANSPARENT);
	//Draw the spot circle
	display.drawCircle(160, 120, 4);
	//Draw the lines
	display.drawHLine(150, 120, 6);
	display.drawHLine(164, 120, 6);
	display.drawVLine(160, 110, 6);
	display.drawVLine(160, 124, 6);
	//Receive object temperature
	if (!save)
		mlx90614GetTemp();
	//Convert to Fahrenheit if needed
	if (tempFormat == 1)
		mlx90614Temp = celciusToFahrenheit(mlx90614Temp);
	//Convert to float with a special method
	char buffer[10];
	floatToChar(buffer, mlx90614Temp);
	display.print(buffer, 145, 140);
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

/* Creates a thermal image and stores it in the array */
void createThermalImg(bool menu) {
	//Receive the temperatures over SPI
	getTemperatures();
	//Find min and max
	if (agcEnabled)
		limitValues();
	//Scale the values
	scaleValues();
	//Scale values and apply gaussian filter
	if ((filterEnabled) || menu)
		gaussianBlur(image, 160, 120, 1.5f, 1);
	//Convert lepton data to RGB565 colors
	convertColors();
}

/* Create and display the thermal image on screen */
void displayThermalImg() {
	//If the image save marker was set
	if (imgSave == 2)
		imgSave = 3;
	//Create the thermal image first
	createThermalImg();
	//Draw thermal image on screen
	if (imgSave != 2) display.writeScreen(image);
	//If the image save marker was set
	if (imgSave == 3)
		imgSave = 1;
}
