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

/* Draws the battery status on the screen */
void drawBatteryStat() {
	//Corner
	display.drawRoundRect(282, 0, 317, 10);
	//Fill background
	display.setColor(127, 127, 127);
	display.fillRoundRect(283, 1, 316, 9);
	display.setColor(VGA_WHITE);
	//Full
	if (batPercentage >= 80)
		display.fillRect(309, 2, 315, 9);
	// 3/4 Full
	if (batPercentage >= 60)
		display.fillRect(301, 2, 307, 9);
	// 1/2 Full
	if (batPercentage >= 40)
		display.fillRect(293, 2, 299, 9);
	//1/4 Full
	if (batPercentage >= 20)
		display.fillRect(285, 2, 291, 9);
}

/* Measure the battery voltage and convert it to percent */
void checkBattery(bool start = false) {
	//Read battery voltage
	float vBat = ((1195 * 1.47 * analogRead(23)) / analogRead(39) / 1000.0);
	//Check if the USB is connected
	float vUSB = ((1195 * 1.47 * analogRead(A14)) / analogRead(39) / 1000.0);
	if (vUSB <= 4.0) {
		vBat += 0.08;
	}
	//Calculate the percentage
	batPercentage = getLipoPerc(vBat);
	//Set the timestamp
	if(start) batRefreshTime = millis() - 60000;
	else batRefreshTime = millis();
}