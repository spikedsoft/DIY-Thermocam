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
	//For Lepton2 sensor
	if (leptonVersion != 1)
		postscale = (float)(pow(dnu / lambda, 2 * numsteps));
	//For Lepton3 sensor
	else
		postscale = (float)(pow(dnu / lambda, numsteps));
	//Filter horizontally along each row
	for (y = 0; y < height; y++) {
		for (step = 0; step < numsteps; step++) {
			ptr = img + width * y;
			ptr[0] *= boundaryscale;
			//Filter rightwards
			for (x = 1; x < width; x++)
				ptr[x] += nu * ptr[x - 1];
			//For Lepton2 sensor
			if (leptonVersion != 1) {
				ptr[x = width - 1] *= boundaryscale;
				//Filter leftwards
				for (; x > 0; x--)
					ptr[x - 1] += nu * ptr[x];
			}
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
			//For Lepton2 sensor
			if (leptonVersion != 1) {
				ptr[i = numpixels - width] *= boundaryscale;
				//Filter upwards
				for (; i > 0; i -= width)
					ptr[i - width] += nu * ptr[i];
			}
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
void getTemperatures(bool save) {
	byte leptonError = 0;
	byte line, segmentNumbers;
	//For Lepton2 sensor, get only one segment per frame
	if (leptonVersion != 1)
		segmentNumbers = 1;
	//For Lepton3 sensor, get four packages per frame
	else
		segmentNumbers = 4;
	//Begin SPI Transmission
	leptonBeginSPI();
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
						leptonEndSPI();
						delay(186);
						leptonBeginSPI();
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
						if ((mlx90614Version == 0) && (leptonVersion != 1)) {
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
						//All other
						else if ((mlx90614Version == 1) && (leptonVersion != 1)) {
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
	leptonEndSPI();
}

/* Scale the values from 0 - 255 */
void scaleValues() {
	//Calculate the scale
	float scale = 255.0 / (maxTemp - minTemp);
	for (int i = 0; i < 19200; i++) {
		//Limit values if not in AGC mode
		if (!agcEnabled) {
			if (image[i] > maxTemp)
				image[i] = maxTemp;
			if (image[i] < minTemp)
				image[i] = minTemp;
		}
		image[i] = (image[i] - minTemp) * scale;
	}
		
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
