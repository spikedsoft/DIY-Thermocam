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
	if (calStatus != 2) 
		calOffset = mlx90614Amb - 204.8;
	//Calculate the temperature in Celcius out of the coefficients
	float temp = (calSlope * rawValue) + calOffset;
	//Convert to Fahrenheit if needed
	if (tempFormat)
		temp = celciusToFahrenheit(temp);
	return temp;
}

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
	sum = sum / 30.5;
	uint16_t avg = (uint16_t)sum;
	return avg;
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

void calibrationProcess() {
	float calMLX90614[100];
	uint16_t calLepton[100];
	//Variable declaration
	float calCorrelation;
	uint16_t average;
	uint16_t average_old = 0;
	float mlx90614_old = 0;
	maxTemp = 0;
	minTemp = 65535;
	do {
		display.print((char*) "Status:  0%", CENTER, 140);
		//Show the screen
		calibrationScreen();
		//Reset counter to zero
		int counter = 0;
		//Perform FFC if shutter is attached
		if (leptonVersion != 2)
			leptonRunCalibration();
		//Get 100 different calibration samples
		while (counter < 100) {
			long timeElapsed = millis();
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
		linreg(100, calLepton, calMLX90614, &calSlope, &calOffset, &calCorrelation);
		calStatus = 2;
		if (calCorrelation < 0.5) {
			if (!calibrationRepeat()) {
				calSlope = 0.025;
				calStatus = 1;
				break;
			}
		}
	} while (calCorrelation < 0.5);
	display.setFont(smallFont);
}

/* Calibration */
bool calibrate() {
	//Still in warmup
	if((millis() - calTimer) < 300000){
		drawMessage((char*) "Please wait 5 minutes after startup!");
		delay(1500);
		return false;
	}
	//If there is a calibration
	else if (calStatus == 2)
		return calibrationChooser();
	//If there is none, do a new one
	else
		calibrationProcess();
	return true;
}