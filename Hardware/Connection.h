/*
* Serial Connection
*
* This file describes the communication protocol for the USB Serial data transmission
*
* Supported devices are: Thermal Viewer, Video Output Module, ThermoVision Software, Serial Terminal
*
*/

/* Defines */

//Start & Stop command
#define CMD_START         100
#define CMD_END	          200

//Serial terminal commands
#define CMD_RAWLIMITS     110
#define CMD_RAWDATA       111
#define CMD_CONFIGDATA    112
#define CMD_VISUALIMG     113
#define CMD_CALIBDATA     114
#define CMD_SPOTTEMP      115
#define CMD_SETTIME       116
#define CMD_TEMPPOINTS    117
#define CMD_LASERTOGGLE   118
#define CMD_LASERSTATE    119
#define CMD_SHUTTERRUN    120
#define CMD_SHUTTERAUTO   121
#define CMD_SHUTTERMANUAL 122
#define CMD_SHUTTERSTATE  123
#define CMD_BATTERYSTATUS 124

//Serial frame commands
#define CMD_RAWFRAME      150
#define CMD_COLORFRAME    151
//Types of frame responses
#define FRAME_CAPTURE     180
#define FRAME_STARTVID    181
#define FRAME_STOPVID     182
#define FRAME_NORMAL      183

/* Variables */

//Command, default send frame
byte sendCmd = FRAME_NORMAL;

/* Methods */

/* Get integer out of a text string */
int getInt(String text)
{
	char temp[6];
	text.toCharArray(temp, 5);
	int x = atoi(temp);
	return x;
}

/* Clear all buffers */
void serialClear() {
	Serial.clearReadError();
	Serial.clearWriteError();
	while (Serial.available() > 0) {
		Serial.read();
	}
}

/* Send the lepton raw limits */
void sendRawLimits() {
	//Send min
	Serial.write((minTemp & 0xFF00) >> 8);
	Serial.write(minTemp & 0x00FF);
	//Send max
	Serial.write((maxTemp & 0xFF00) >> 8);
	Serial.write(maxTemp & 0x00FF);
}

/* Send the lepton raw data*/
void sendRawData(bool color = false) {
	uint16_t result;

	//For the Lepton2 sensor, write 4800 raw values
	if ((leptonVersion != leptonVersion_3_Shutter) && (!color)) {
		for (int line = 0; line < 60; line++) {
			for (int column = 0; column < 80; column++) {
				result = image[(line * 2 * 160) + (column * 2)];
				Serial.write((result & 0xFF00) >> 8);
				Serial.write(result & 0x00FF);
			}
		}
	}
	//For the Lepton3 sensor, write 19200 raw values
	else {
		for (int i = 0; i < 19200; i++) {
			Serial.write((image[i] & 0xFF00) >> 8);
			Serial.write(image[i] & 0x00FF);
		}
	}
}

/* Sends the configuration data */
void sendConfigData() {
	//Lepton version
	Serial.write(leptonVersion);
	//Rotation
	Serial.write(rotationEnabled);
	//Send color scheme
	Serial.write(colorScheme);
	//Send the temperature format
	Serial.write(tempFormat);
	//Send the show spot attribute
	Serial.write(spotEnabled);
	//Send the show colorbar attribute
	if (calStatus == cal_warmup)
		Serial.write((byte)0);
	else
		Serial.write(colorbarEnabled);
	//Send the temperature points enabled attribute
	if (calStatus == cal_warmup)
		Serial.write((byte)0);
	else
		Serial.write(pointsEnabled);
	//Send adjust bar allowed
	Serial.write((agcEnabled) && (!limitsLocked));
}

/* Sends the visual image as JPEG */
void sendVisualImg() {
	captureVisualImage();
	transferVisualImage();
}

/* Sends the calibration data */
void sendCalibrationData() {
	uint8_t farray[4];
	//Send the calibration offset first
	floatToBytes(farray, (float)calOffset);
	for (int i = 0; i < 4; i++)
		Serial.write(farray[i]);
	//Send the calibration slope
	floatToBytes(farray, (float)calSlope);
	for (int i = 0; i < 4; i++)
		Serial.write(farray[i]);
}

/* Sends the spot temp*/
void sendSpotTemp() {
	uint8_t farray[4];
	floatToBytes(farray, mlx90614Temp);
	for (int i = 0; i < 4; i++)
		Serial.write(farray[i]);
}

/* Change the color scheme */
void changeColorScheme() {
	if (colorScheme < (colorSchemeTotal - 1))
		colorScheme++;
	else if (colorScheme == (colorSchemeTotal - 1))
		colorScheme = 0;
}

/* Sets the time */
void setTime() {
	//Wait for time string, maximum 1 second
	uint32_t timer = millis();
	while (!Serial.available() && ((millis() - timer) < 1000));
	//If there was no timestring
	if (Serial.available() == 0)
		return;
	//Read time
	String dateIn = Serial.readString();
	//Check if valid
	if (getInt(dateIn.substring(0, 4) >= 2016)) {
		//Set the clock
		setTime(getInt(dateIn.substring(11, 13)), getInt(dateIn.substring(14, 16)), getInt(dateIn.substring(17, 19)),
			getInt(dateIn.substring(8, 10)), getInt(dateIn.substring(5, 7)), getInt(dateIn.substring(0, 4)));
		//Set the RTC
		Teensy3Clock.set(now());
	}
}

/* Send the temperature points */
void sendTempPoints() {
	for (int i = 0; i < 192; i++) {
		Serial.write((showTemp[i] & 0xFF00) >> 8);
		Serial.write(showTemp[i] & 0x00FF);
	}
}

/* Send the laser state */
void sendLaserState() {
	Serial.write(laserEnabled);
}

/* Send the shutter mode */
void sendShutterMode() {
	Serial.write(shutterMode);
}

/* Send the battery status in percentage */
void sendBatteryStatus() {
	Serial.write(batPercentage);
}

/* Sends a raw frame */
void sendFrame(bool color) {
	Serial.write(sendCmd);
	Serial.flush();
	//Send frame
	if (sendCmd == FRAME_NORMAL) {
		//Clear all serial buffers
		serialClear();
		//Convert to colors
		if (color) {
			//Scale Values
			scaleValues();
			//Apply box blur
			boxFilter();
			//Convert to RGB565
			convertColors();
		}
		//Send raw data
		sendRawData(color);
		//Send limits
		sendRawLimits();
		//Send spot temp
		sendSpotTemp();
		//Send calibration data
		sendCalibrationData();
		//Send temperature points
		sendTempPoints();
	}
	//Switch back to send frame the next time
	else
		sendCmd = FRAME_NORMAL;
}

/* Evaluates commands from the serial port*/
bool serialHandler() {
	//Read command from Serial Port
	byte recCmd = Serial.read();

	//Decide what to do
	switch (recCmd) {
		//Send raw limits
	case CMD_RAWLIMITS:
		sendRawLimits();
		break;
		//Send raw data
	case CMD_RAWDATA:
		sendRawData();
		break;
		//Send config data
	case CMD_CONFIGDATA:
		sendConfigData();
		break;
		//Send visual image
	case CMD_VISUALIMG:
		sendVisualImg();
		break;
		//Send calibration data
	case CMD_CALIBDATA:
		sendCalibrationData();
		break;
		//Send spot temp
	case CMD_SPOTTEMP:
		sendSpotTemp();
		break;
		//Change time
	case CMD_SETTIME:
		setTime();
		//Send ACK
		Serial.write(CMD_SETTIME);
		break;
		//Send temperature points
	case CMD_TEMPPOINTS:
		sendTempPoints();
		break;
		//Toggle laser
	case CMD_LASERTOGGLE:
		toggleLaser();
		//Send ACK
		Serial.write(CMD_LASERTOGGLE);
		break;
		//Send laser state
	case CMD_LASERSTATE:
		sendLaserState();
		break;
		//Run the shutter
	case CMD_SHUTTERRUN:
		leptonRunCalibration();
		//Send ACK
		Serial.write(CMD_SHUTTERRUN);
		break;
		//Set shutter mode to manual
	case CMD_SHUTTERMANUAL:
		leptonSetShutterMode(false);
		//Send ACK
		Serial.write(CMD_SHUTTERMANUAL);
		break;
		//Set shutter mode to auto
	case CMD_SHUTTERAUTO:
		leptonSetShutterMode(true);
		//Send ACK
		Serial.write(CMD_SHUTTERAUTO);
		break;
		//Send the shutter mode
	case CMD_SHUTTERSTATE:
		sendShutterMode();
		break;
		//Send battery status
	case CMD_BATTERYSTATUS:
		sendBatteryStatus();
		break;
		//Send raw frame
	case CMD_RAWFRAME:
		sendFrame(false);
		break;
		//Send color frame
	case CMD_COLORFRAME:
		sendFrame(true);
		break;
		//End connection
	case CMD_END:
		return true;
		break;
	}
	Serial.flush();
	return false;
}

/* Evaluate button presses */
void buttonHandler() {
	// Count the time to choose selection
	long startTime = millis();
	long endTime = millis() - startTime;
	while ((extButtonPressed()) && (endTime <= 1000))
		endTime = millis() - startTime;
	endTime = millis() - startTime;
	//Short press - request to save an image
	if ((endTime < 1000) && (sendCmd == FRAME_NORMAL)) {
		sendCmd = FRAME_CAPTURE;
	}
	//Long press - request to start or stop a video
	else {
		//Start video
		if ((videoSave == false) && (sendCmd != FRAME_STOPVID)) {
			sendCmd = FRAME_STARTVID;
			videoSave = true;
			while (extButtonPressed());
		}
		//Stop video
		if ((videoSave == true) && (sendCmd != FRAME_STARTVID)) {
			sendCmd = FRAME_STOPVID;
			videoSave = false;
			while (extButtonPressed());
		}
	}
}

/* Go into video output mode and wait for connected module */
void serialOutput() {
	//Send the frames
	while (true) {
		//Abort transmission
		if (touch.touched())
			break;
		//Check warmup status
		checkWarmup();

		//Get the temps
		getTemperatures();
		//Compensate calibration with object temp
		compensateCalib();
		//Refresh the temp points if enabled
		if (pointsEnabled)
			refreshTempPoints();
		//Find min and max if not in manual mode and limits not locked
		if ((agcEnabled) && (!limitsLocked)) {
			//Limit values if we are in the menu or not in cold/hot mode
			if ((colorScheme != colorScheme_coldest) && (colorScheme != colorScheme_hottest))
				limitValues();
		}

		//Check button press if not in terminal mode
		if (extButtonPressed())
			buttonHandler();

		//Check for serial commands
		if (Serial.available() > 0) {
			//Check for exit
			if (serialHandler())
				break;
		}
	}
}

/* Tries to establish a connection to a thermal viewer or video output module*/
void serialConnect() {
	//Disable interrupts
	detachInterrupts();
	//Set camera resolution to medium
	changeCamRes(VC0706_320x240);

	//Show message
	drawMessage((char*)"Serial connection detected !");
	display.print((char*) "Touch screen to return", CENTER, 170);
	delay(1000);

	//Disable screen backlight
	disableScreenLight();

	//Turn laser off if enabled
	if (laserEnabled)
		toggleLaser();

	//Send ACK for Start
	Serial.write(CMD_START);

	//Go to the serial output
	serialOutput();

	//Send ACK for End
	Serial.write(CMD_END);

	//Re-Enable display backlight
	enableScreenLight();

	//Show message
	drawMessage((char*)"Connection ended, return..");
	delay(1000);

	//Clear all serial buffers
	serialClear();

	//Change camera resolution back
	if (displayMode == displayMode_thermal)
		changeCamRes(VC0706_640x480);
	else
		changeCamRes(VC0706_160x120);

	//Turn laser off if enabled
	if (laserEnabled)
		toggleLaser();

	//Switch back to auto shutter if manual used
	if (shutterMode == shutterMode_manual)
		leptonSetShutterMode(true);

	//Re-Enable interrupts
	attachInterrupts();
	//Disable video mode
	videoSave = false;
}