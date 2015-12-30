/*
* Calibration functions for the raw to temperature conversion
*/

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
	float result;
	//Normal calibration
	if (calibrationDone) {
		//float result = calSlope * rawValue + calOffset;
		result = 1428.0 / log((calSlope / (rawValue - calOffset)) + 1.0);
		//Convert Kelvin to Celcius
		result -= 273.15;
	}
	//Quick calibration
	else {
		result = (0.022 * rawValue) + quickCalOffset;
	}
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


/* Do a quick calibration for the color bar */
void quickCalibration() {
	//Allocate space for Lepton array
	image = (unsigned short*)calloc(19200, sizeof(unsigned short));
	//Storage for sums
	long rawSum = 0;
	double absoluteSum = 0;
	//Repeat procedure 10 times
	for (int i = 0; i < 10; i++) {
		getTemperatures();
		rawSum += calcAverage();
		absoluteSum += mlx90614GetTemp();

	}
	//Divide through 10
	int rawValue = (int)rawSum / 10.0;
	float absoluteValue = absoluteSum / 10.0;
	//Calculate offset
	quickCalOffset = absoluteValue - (rawValue * 0.022);
	//De-allocate space again
	free(image);
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