/*
* Functions to convert Lepton raw values to absolute values
*/

/* Converts a given Temperature in Celcius to Fahrenheit */
float celciusToFahrenheit(float Tc) {
	float Tf = ((float) 9.0 / 5.0) * Tc + 32.0;
	return (Tf);
}

/* Function to calculate temperature out of Lepton value */
float calFunction(uint16_t rawValue) {
	//Calculate offset out of ambient temp when no calib is done
	if (calStatus != cal_manual)
		calOffset = mlx90614Amb - (calSlope * 8192) + calComp;
	//Calculate the temperature in Celcius out of the coefficients
	float temp = (calSlope * rawValue) + calOffset;
	//Convert to Fahrenheit if needed
	if (tempFormat == tempFormat_fahrenheit)
		temp = celciusToFahrenheit(temp);
	return temp;
}

/* Calculates the average of the 196 (14x14) pixels in the middle */
uint16_t calcAverage() {
	int sum = 0;
	for (byte vert = 52; vert < 66; vert++) {
		for (byte horiz = 72; horiz < 86; horiz++) {
			uint16_t val = image[(vert * 160) + horiz];
			//If one of the values contains hotter or colder values than the lepton can handle
			if ((val == 16383) || (val == 0))
				//Do not use that calibration set!
				return 0;
			sum += val;
		}
	}
	uint16_t avg = (uint16_t)(sum / 196.0);
	return avg;
}

/* Compensate the calibration with object temp */
void compensateCalib() {
	//Refresh MLX90614 ambient temp
	mlx90614GetAmb();
	///Refresh object temperature
	mlx90614GetTemp();
	//Convert to Fahrenheit if needed
	if (tempFormat == tempFormat_fahrenheit)
		mlx90614Temp = celciusToFahrenheit(mlx90614Temp);
	//Apply compensation if AGC enabled, no limited locked and standard calib
	if ((agcEnabled) && (!limitsLocked) && (calStatus == cal_standard)) {
		//Calculate min & max
		float min = calFunction(minTemp);
		float max = calFunction(maxTemp);
		//If spot temp is lower than current minimum by 2 degree, lower minimum
		if ((mlx90614Temp < (min - 1)) && (colorScheme != colorScheme_hottest))
			calComp = mlx90614Temp - min;
		//If spot temp is higher than current maximum by 2 degree, raise maximum
		else if ((mlx90614Temp > (max + 1)) && (colorScheme != colorScheme_coldest))
			calComp = mlx90614Temp - max;
	}
	//Calculate offset out of ambient temp when no calib is done
	if (calStatus != cal_standard)
		calOffset = mlx90614Amb - (calSlope * 8192) + calComp;
}

/* Checks if the calibration warmup is done */
void checkWarmup() {
	//Activate the calibration after a warmup time of 60s
	if (((calStatus == cal_warmup) && (millis() - calTimer > 60000))) {
		//Perform FFC if shutter is attached
		if (leptonVersion != leptonVersion_2_NoShutter)
			leptonRunCalibration();
		//Set calibration status to standard for new HW
		if (mlx90614Version == mlx90614Version_new)
			calStatus = cal_standard;
		//Set calibration status to manual for old HW
		else
			calStatus = cal_manual;
	}
}

/* Help function for least suqare fit */
inline static double sqr(double x) {
	return x*x;
}

/* Least square fit */
int linreg(int n, const uint16_t x[], const float y[], float* m, float* b, float* r)
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

/* Run the calibration process */
void calibrationProcess(bool firstStart) {
	//Variables
	float calMLX90614[100];
	uint16_t calLepton[100];
	float calCorrelation;
	char result[30];
	uint16_t average;
	uint16_t average_old = 0;
	float mlx90614_old = 0;
	maxTemp = 0;
	minTemp = 65535;

	//Repeat as long as there is a good calibration
	do {
		//Show the screen
		calibrationScreen(firstStart);
		//Reset counter to zero
		int counter = 0;

		//Perform FFC if shutter is attached
		if (leptonVersion != leptonVersion_2_NoShutter)
			leptonRunCalibration();

		//Get 100 different calibration samples
		while (counter < 100) {
			long timeElapsed = millis();

			//Repeat measurement as long as there is no new average
			do {
				//Safe delay for bad PCB routing
				delay(10);
				//Get temp and calc average
				getTemperatures();
				average = calcAverage();
			} while ((average == average_old) || (average == 0));
			average_old = average;

			//If the temperature changes too much, do not take that measurement
			if (abs(mlx90614GetTemp() - mlx90614_old) < 10) {
				//Store values
				calLepton[counter] = average;
				calMLX90614[counter] = mlx90614Temp;
				//Find minimum and maximum value
				if (average > maxTemp)
					maxTemp = average;
				if (average < minTemp)
					minTemp = average;
				if ((counter % 10) == 0) {
					char buffer[20];
					sprintf(buffer, "Status: %2d%%", counter);
					display.print(buffer, CENTER, 140);
				}
				counter++;
			}
			mlx90614_old = mlx90614Temp;

			//Wait at least 111ms between two measurements (9Hz)
			while ((millis() - timeElapsed) < 111);
			//If the user wants to abort and is not in first start
			if ((touch.touched() == true) && (firstStart == false)) {
				int pressedButton = touchButtons.checkButtons(true);
				//Abort
				if (pressedButton == 0) {
					return;
				}
			}
		}

		//Calculate the calibration formula with least square fit
		linreg(100, calLepton, calMLX90614, &calSlope, &calOffset, &calCorrelation);
		//Set calibration to manual
		calStatus = cal_manual;
		//Set compensation to zero
		calComp = 0;

		//In case the calibration was not good, ask to repeat
		if (calCorrelation < 0.5) {
			//When in first start mode
			if (firstStart) {
				drawMessage((char*) "Bad calibration, try again!");
				delay(1000);
			}
			//If the user does not want to repeat, discard
			else if (!calibrationRepeat()) {
				calSlope = cal_stdSlope;
				calStatus = cal_standard;
				break;
			}
		}
	} while (calCorrelation < 0.5);

	//Show the result
	sprintf(result, "Slope: %1.4f, offset: %.1f", calSlope, calOffset);
	drawMessage(result);
	delay(2000);

	//Save calibration to EEPROM
	storeCalibration();
	//Show message if not in first start menu
	if (firstStart == false) {
		drawMessage((char*) "Calibration written to EEPROM!");
		delay(1000);
	}
	//Restore old font
	display.setFont(smallFont);
}

/* Calibration */
bool calibrate() {
	//Still in warmup
	if (calStatus == cal_warmup) {
		drawMessage((char*) "Please wait for sensor warmup!");
		delay(1500);
		return false;
	}
	//If there is a calibration
	else if (calStatus == cal_manual)
		return calibrationChooser();
	//If there is none, do a new one
	else
		calibrationProcess();
	return true;
}