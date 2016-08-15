/*
* Measure the battery status
*/

/* A method to calculate the lipo percentage out of its voltage */
int getLipoPerc(float vol) {
	if (vol >= 4.10)
		return 100;
	else if (vol >= 4.05)
		return 95;
	else if (vol >= 4.00)
		return 90;
	else if (vol >= 3.96)
		return 85;
	else if (vol >= 3.93)
		return 80;
	else if (vol >= 3.91)
		return 75;
	else if (vol >= 3.88)
		return 70;
	else if (vol >= 3.86)
		return 65;
	else if (vol >= 3.84)
		return 60;
	else if (vol >= 3.82)
		return 55;
	else if (vol >= 3.80)
		return 50;
	else if (vol >= 3.78)
		return 45;
	else if (vol >= 3.76)
		return 40;
	else if (vol >= 3.74)
		return 35;
	else if (vol >= 3.73)
		return 30;
	else if (vol >= 3.71)
		return 25;
	else if (vol >= 3.70)
		return 20;
	else if (vol >= 3.68)
		return 15;
	else if (vol >= 3.67)
		return 10;
	else if (vol >= 3.65)
		return 5;
	else if (vol >= 3.00)
		return 0;
	else
		return -1;
}

/* Measure the battery voltage and convert it to percent */
void checkBattery(bool start = false) {
	//Read battery voltage
	float vBat = (batMeasure->analogRead(pin_bat_measure) * 1.5 * 3.3) / batMeasure->getMaxValue(ADC_0);
	//Check if the USB is connected
	float vUSB = (batMeasure->analogRead(pin_usb_measure) * 1.5 * 3.3) / batMeasure->getMaxValue(ADC_0);
	//If the battery gauge is not working
	if ((vBat == -1) && (vUSB <= 1.0)) {
		drawMessage((char*) "Battery gauge is not working!");
		delay(1000);
		setDiagnostic(diag_bat);
	}
	//If not connected, add some value to correct it
	if (vUSB <= 1.0)
		vBat += 0.05;
	//Calculate the percentage
	batPercentage = getLipoPerc(vBat);
}