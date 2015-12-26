/*
* Main functions in the live mode
*/

/* Includes */
#include "Create.h"
#include "Load.h"
#include "Save.h"

/* Variables */
float weightArray[] = {
	0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25,
	0.25, 0.50, 0.50, 0.50, 0.50, 0.50, 0.50, 0.25,
	0.25, 0.50, 0.75, 0.75, 0.75, 0.75, 0.50, 0.25,
	0.25, 0.50, 0.75, 1.00, 1.00, 0.75, 0.50, 0.25,
	0.25, 0.50, 0.75, 1.00, 1.00, 0.75, 0.50, 0.25,
	0.25, 0.50, 0.75, 0.75, 0.75, 0.75, 0.50, 0.25,
	0.25, 0.50, 0.50, 0.50, 0.50, 0.50, 0.50, 0.25,
	0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25, 0.25
};

/* Methods */

/* Converts a given Temperature in Celcius to Fahrenheit */
float celciusToFahrenheit(float Tc) {
	float Tf = ((float) 9.0 / 5.0) * Tc + 32.0;
	return (Tf);
}

/* Function to calculate temperature out of Lepton value */
float calFunction(uint16_t rawValue) {
	//Calc out of the formula
	//float result = calSlope * rawValue + calOffset;
	float result = 1428.0 / log((calSlope / (rawValue - calOffset)) + 1.0);
	//Convert Kelvin to Celcius
	result -= 273.15;
	//Convert to Fahrenheit if needed
	if (tempFormat)
		result = celciusToFahrenheit(result);
	return result;
}

/* Calculates the average of the 64 pixels in the middle */
uint16_t calcAverage() {
	float sum = 0.0;
	for (byte vert = 0; vert < 8; vert++) {
		for (byte horiz = 0; horiz < 8; horiz++) {
			uint16_t val = image[((vert + 26) * 160) + (horiz + 36) * 2];
			//If one of the values contains hotter or colder values than the lepton can handle
			if ((val == 16383) || (val == 0))
				//Do not use that calibration set!
				return 0;
			sum = sum + (val * weightArray[(vert * 8) + horiz]);
		}
	}
	sum = sum / 30.0;
	uint16_t avg = (uint16_t)(sum + 0.5);
	return avg;
}

/* Help function for least suqare fit */
inline static double sqr(double x) {
	return x*x;
}

/* Least square fit */
int linreg(int n, const double x[], const double y[], double* m, double* b, double* r)
{
	double   sumx = 0.0;
	double   sumx2 = 0.0;
	double   sumxy = 0.0;
	double   sumy = 0.0;
	double   sumy2 = 0.0;
	for (int i = 0; i < n; i++)
	{
		sumx += x[i];
		sumx2 += sqr(x[i]);
		sumxy += x[i] * y[i];
		sumy += y[i];
		sumy2 += sqr(y[i]);
	}
	double denom = (n * sumx2 - sqr(sumx));
	if (denom == 0) {
		//singular matrix. can't solve the problem.
		*m = 0;
		*b = 0;
		*r = 0;
		return 1;
	}
	*m = (n * sumxy - sumx * sumy) / denom;
	*b = (sumy * sumx2 - sumx * sumxy) / denom;
	if (r != NULL) {
		*r = (sumxy - sumx * sumy / n) /
			sqrt((sumx2 - sqr(sumx) / n) *
				(sumy2 - sqr(sumy) / n));
	}
	return 0;
}

/* Calibration Proccess */
void calibrationProccess() {
	//Variable declaration
	double cal_Correlation;
	double cal_MLX90614[100];
	double cal_Lepton[100];
	uint16_t average;
	uint16_t average_old = 0;
	float mlx90614_old = 0;
	maxTemp = 0;
	minTemp = 65535;
	do {
		//Show the screen
		calibrateScreen();
		int counter = 0;
		//Perform FFC
		leptonRunFFC();
		//Get 100 different calibration samples
		while (counter < 100) {
			long timeElapsed = millis();
			do {
				getTemperatures();
				average = calcAverage();
			} while ((average == average_old) || (average == 0));
			average_old = average;
			//If the temperature changes too much, do not take that measurement
			if (abs(mlx90614GetTemp() - mlx90614_old) < 10) {
				//Convert Object temp to Fahrenheit if needed
				if (tempFormat == 1)
					mlx90614Temp = celciusToFahrenheit(mlx90614Temp);
				//Store values
				cal_Lepton[counter] = average;
				cal_MLX90614[counter] = 1.0 / (exp(1428.0 / (mlx90614Temp + 273.15)) - 1.0);
				//Find minimum and maximum value
				if (average > maxTemp)
					maxTemp = average;
				if (average < minTemp)
					minTemp = average;
				if ((counter % 10) == 0) {
					char buffer[4];
					sprintf(buffer, "Status: %2d%%", counter);
					display.print(buffer, CENTER, 140);
				}
				counter++;
			}
			mlx90614_old = mlx90614Temp;
			//wait at least 111ms between two measurements (9Hz)
			while ((millis() - timeElapsed) < 111);
			//If the user wants to abort
			if (touch.touched() == true) {
				int pressedButton = touchButtons.checkButtons(true);
				//Abort
				if (pressedButton == 0) {
					return;
				}
			}
		}
		//Calculate the calibration formula
		linreg(100, cal_MLX90614, cal_Lepton, &calSlope, &calOffset, &cal_Correlation);
		//Show error message if calibration was not good
		if (cal_Correlation < 0.5) {
			if (!calibrationRepeat())
				return;
		}
	} while (cal_Correlation < 0.5);
	//Set calibration to done
	calibrationDone = true;
	//Restore old font
	display.setFont(smallFont);
}

/* Calibration */
bool calibrate() {
	//If there is a calibration
	if (calibrationDone)
		return calibrationChooser();
	//If there is none, do a new one
	else {
		calibrationProccess();
		return true;
	}
}

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

/* Toggles the spot display */
void toggleSpot() {
	if (spotEnabled) {
		spotEnabled = false;
	}
	else {
		spotEnabled = true;
	}
}

/* Toggles the filter */
void toggleFilter() {
	if (filterEnabled) {
		filterEnabled = false;
	}
	else {
		filterEnabled = true;
	}
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

/* Init procedure for the live mode */
void liveModeInit() {
	//Select color scheme
	selectColorScheme();
	//Attach the interrupts
	attachInterrupts();
}

void liveModeExit() {
	//Detach the interrupts
	detachInterrupts();
	//Open the main menu
	mainMenu();
}

/* Main entry point for the live mode */
void liveMode() {
	//Init
	liveModeInit();
	//Allocate space
	image = (unsigned short*)calloc(19200, sizeof(unsigned short));
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
		if (calibrationDone)
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
	//Deallocate space
	free(image);
	//Exit
	liveModeExit();
}