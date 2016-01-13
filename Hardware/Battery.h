/*
* Measure the battery status
*/

/* A method to calculate the lipo percentage out of its voltage */
int getLipoPerc(float vol) {
	if (vol >= 4.20)
		return 100;
	else if (vol >= 4.13)
		return 95;
	else if (vol >= 4.09)
		return 90;
	else if (vol >= 4.05)
		return 85;
	else if (vol >= 4.00)
		return 80;
	else if (vol >= 3.98)
		return 75;
	else if (vol >= 3.94)
		return 70;
	else if (vol >= 3.90)
		return 65;
	else if (vol >= 3.87)
		return 60;
	else if (vol >= 3.85)
		return 55;
	else if (vol >= 3.82)
		return 50;
	else if (vol >= 3.81)
		return 45;
	else if (vol >= 3.79)
		return 40;
	else if (vol >= 3.78)
		return 35;
	else if (vol >= 3.76)
		return 30;
	else if (vol >= 3.74)
		return 25;
	else if (vol >= 3.72)
		return 20;
	else if (vol >= 3.70)
		return 15;
	else if (vol >= 3.68)
		return 10;
	else if (vol >= 3.54)
		return 5;
	else if (vol >= 3.20)
		return 0;
	else
		return -1;
}

/* Measure the battery voltage and convert it to percent */
void checkBattery(bool start = false) {
	//Read battery voltage
	float vBat = (batMeasure->analogRead(pin_bat_measure) * 1.47 * 3.3) / batMeasure->getMaxValue(ADC_0) + 0.25;
	//Check if the USB is connected
	float vUSB = (batMeasure->analogRead(pin_usb_measure) * 1.47 * 3.3) / batMeasure->getMaxValue(ADC_0);
	//If not connected, add some value to correct it
	if (vUSB <= 4.0)
		vBat += 0.1;
	//Calculate the percentage
	batPercentage = getLipoPerc(vBat);
	//Set the timestamp
	if(start) batRefreshTime = millis() - 60000;
	else batRefreshTime = millis();
}