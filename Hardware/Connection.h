/*
* Serial Connection
*
* This file describes the communication protocol for the USB Serial data transmission
*
* Supported devices are: Thermal Viewer, Video Output Module, ThermoVision Software, Serial Terminal
*
*/

/* Defines */

//Commands for the serial terminal
#define CMD_TERMINAL_CONNECT 't'
#define CMD_TERMINAL_LEPTONVERSION 'v'
#define CMD_TERMINAL_ROTATION 'r'
#define CMD_TERMINAL_SPOTTEMP 's'
#define CMD_TERMINAL_AMBIENTTEMP 'a'
#define CMD_TERMINAL_RAWDATA 'd'
#define CMD_TERMINAL_CALIBDATA 'c'
#define CMD_TERMINAL_END 'e'

//Commands for the other devices
#define CMD_THERMOVISION 'W'
#define CMD_VERSION 'V'
#define CMD_VIDEOMODULE 'U'
#define CMD_THERMALVIEWER 'T'
#define CMD_START 'S'
#define CMD_ROTATED 'R'
#define CMD_SPOT 'Q'
#define CMD_GET 'G'
#define CMD_END 'E'
#define CMD_COLORSCHEME 'C'

//Commands that are send to the device
#define START_SESSION 0
#define CAP_IMG 100
#define START_VID 150
#define STOP_VID 200
#define SEND_FRAME 250

//List of possible devices
#define videoOut_thermalViewer 0
#define videoOut_externalModule 1
#define videoOut_thermoVision 2
#define videoOut_serialTerminal 3

/* Variables */

//Command, default send frame
byte sendCmd = SEND_FRAME;
//Video out type
byte videoOutType;


/* Methods */

/* Process time sync messages from the serial port */
unsigned long processSyncMessage() {
	unsigned long pctime = 0L;
	const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013 

	if (Serial.find((char*)"T")) {
		pctime = Serial.parseInt();
		return pctime;
		if (pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
			pctime = 0L; // return 0 to indicate that the time is not valid
		}
	}
	return pctime;
}

/* Clear all buffers */
void serialClear() {
	Serial.clearReadError();
	Serial.clearWriteError();
	while (Serial.available() > 0) {
		Serial.read();
	}
}

/* Send the config data */
void sendConfigData() {
	uint8_t farray[4];
	//For thermoVision application, send only slope & offset
	if (videoOutType != videoOut_thermoVision) {
		//Send min
		Serial.write((minTemp & 0xFF00) >> 8);
		Serial.write(minTemp & 0x00FF);
		Serial.write((maxTemp & 0xFF00) >> 8);
		Serial.write(maxTemp & 0x00FF);
		//Send object temp
		mlx90614GetTemp();
		floatToBytes(farray, mlx90614Temp);
		for (int i = 0; i < 4; i++)
			Serial.write(farray[i]);
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
	}
	//Send the calibration offset
	if (calStatus != cal_manual)
		calOffset = mlx90614GetAmb() - (calSlope * 8192);
	floatToBytes(farray, (float)calOffset);
	for (int i = 0; i < 4; i++)
		Serial.write(farray[i]);
	//Send the calibration slope
	floatToBytes(farray, (float)calSlope);
	for (int i = 0; i < 4; i++)
		Serial.write(farray[i]);
	//For thermoVision application, send only slope & offset
	if (videoOutType != videoOut_thermoVision) {
		//Send the temperature points if enabled
		if (pointsEnabled) {
			for (int i = 0; i < 192; i++) {
				Serial.write((showTemp[i] & 0xFF00) >> 8);
				Serial.write(showTemp[i] & 0x00FF);
			}
		}
		else {
			//Write dummy data if not enabled
			for (int i = 0; i < 384; i++) {
				Serial.write((byte)0);
			}
		}
		//Send adjust bar allowed
		Serial.write((agcEnabled) && (!limitsLocked));
		//Send check marker
		Serial.write('#');
	}
}

/* Send the Lepton data*/
void sendLeptonData() {
	byte MSB, LSB;
	//Send Lepton2 data
	if (leptonVersion != leptonVersion_3_Shutter) {
		for (int i = 0; i < 4800; i++) {
			MSB = (byte)(rawValues[i] & 0xff);
			LSB = (byte)((rawValues[i] >> 8) & 0xff);
			Serial.write(LSB);
			Serial.write(MSB);
		}
	}
	//Send Lepton3 data
	else {
		for (int i = 0; i < 19200; i++) {
			MSB = (byte)(image[i] & 0xff);
			LSB = (byte)((image[i] >> 8) & 0xff);
			Serial.write(LSB);
			Serial.write(MSB);
		}
	}
}

/* Evaluates commands from the terminal */
bool serialTerminal() {
	uint8_t farray[4];

	//Read command from Serial Port
	byte recCmd = Serial.read();

	//Decide what to do
	switch (recCmd) {
		//Get Lepton version
	case CMD_TERMINAL_LEPTONVERSION:
		Serial.write(leptonVersion);
		break;
		//Get the rotation
	case CMD_TERMINAL_ROTATION:
		Serial.write(rotationEnabled);
		break;
		//Get ambient temp
	case CMD_TERMINAL_AMBIENTTEMP:
		mlx90614GetAmb();
		floatToBytes(farray, mlx90614Amb);
		for (int i = 0; i < 4; i++)
			Serial.write(farray[i]);
		break;
		//Get spot object temp
	case CMD_TERMINAL_SPOTTEMP:
		mlx90614GetTemp();
		floatToBytes(farray, mlx90614Temp);
		for (int i = 0; i < 4; i++)
			Serial.write(farray[i]);
		break;
		//Get calibration data
	case CMD_TERMINAL_CALIBDATA:
		if (calStatus != cal_manual)
			calOffset = mlx90614Amb - (calSlope * 8192);
		floatToBytes(farray, calOffset);
		for (int i = 0; i < 4; i++)
			Serial.write(farray[i]);
		floatToBytes(farray, calSlope);
		for (int i = 0; i < 4; i++)
			Serial.write(farray[i]);
		break;
		//Send the lepton raw data
	case CMD_TERMINAL_RAWDATA:
		sendLeptonData();
		break;
		//End connection
	case CMD_TERMINAL_END:
		return true;
		break;
	//TODO: Implement the rest of the commands	
	}
	Serial.flush();
	return false;
}

/* Evaluates commands from the serial port*/
bool serialHandler() {
	uint8_t farray[4];

	//Read command from Serial Port
	byte recCmd = Serial.read();

	//Decide what to do
	switch (recCmd) {
		//Get frame command
	case CMD_GET:
		//Send response (Picture/Video/Normal)
		Serial.write(sendCmd);
		Serial.flush();
		//Send frame
		if (sendCmd == SEND_FRAME) {
			//Clear all serial buffers
			serialClear();
			//Send Lepton data
			sendLeptonData();
			//Send config data
			sendConfigData();
		}
		//Switch back to send frame the next time
		else
			sendCmd = SEND_FRAME;
		break;
		//End connection
	case CMD_END:
		return true;
		break;
		//Change color scheme
	case CMD_COLORSCHEME:
		if (colorScheme < (colorSchemeTotal - 1))
			colorScheme++;
		else if (colorScheme == (colorSchemeTotal - 1))
			colorScheme = 0;
		break;
		//Get rotation
	case CMD_ROTATED:
		Serial.write(rotationEnabled);
		break;
		//Send lepton version
	case CMD_VERSION:
		Serial.write(leptonVersion);
		break;
		//Get spot temp
	case CMD_SPOT:
		mlx90614GetTemp();
		floatToBytes(farray, mlx90614Temp);
		for (int i = 0; i < 4; i++)
			Serial.write(farray[i]);
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
	if (endTime < 1000) {
		sendCmd = CAP_IMG;
	}
	//Long press - request to start or stop a video
	else {
		//Start video
		if (!videoSave) {
			sendCmd = START_VID;
			videoSave = true;
		}
		//Stop video
		else {
			sendCmd = STOP_VID;
			videoSave = false;
		}
	}
}

/* Go into video output mode and wait for connected module */
void videoOutput() {
	//Disable screen backlight
	disableScreenLight();
	//Send the frames
	while (true) {
		//Abort transmission
		if (touch.touched())
			break;
		//Get the temps
		getTemperatures(true);
		//Refresh the temp points if enabled
		if (pointsEnabled)
			refreshTempPoints(true);
		//For 160x120 Lepton3
		if (leptonVersion == leptonVersion_3_Shutter) {
			//Find min and max if required
			if ((agcEnabled) && (!limitsLocked) && (colorScheme != colorScheme_coldest) && (colorScheme != colorScheme_hottest))
				limitValues();
			//Convert to colors for video out module
			if (videoOutType == videoOut_externalModule) {
				scaleValues();
				convertColors();
			}
		}
		//For 80x60 Lepton2
		else {
			//Find min and max if required
			if ((agcEnabled) && (!limitsLocked) && (colorScheme != colorScheme_coldest) && (colorScheme != colorScheme_hottest))
				limitValues(true);
			//Convert to colors for video out module
			if(videoOutType == videoOut_externalModule) {
				scaleValues(true);
				convertColors(true);
			}
		}
		//Activate the calibration after warmup
		if (calStatus == cal_warmup) {
			if (millis() - calTimer > 60000) {
				//Perform FFC if shutter is attached
				if (leptonVersion != leptonVersion_2_NoShutter)
					leptonRunCalibration();
				calStatus = cal_standard;
			}
		}
		//Check buttton press if not in terminal mode
		if (extButtonPressed() && (videoOutType != videoOut_serialTerminal))
			buttonHandler();
		//Serial command check
		if (Serial.available() > 0) {
			//Terminal mode
			if (videoOutType == videoOut_serialTerminal) {
				if (serialTerminal())
					break;
			}
			//All other
			else {
				if (serialHandler())
					break;
			}
		}
	}
	//Enable display backlight
	enableScreenLight();
}

/* Get integer out of a text string */
int getInt(String text)
{
	char temp[6];
	text.toCharArray(temp, 5);
	int x = atoi(temp);
	return x;
}

/* Tries to establish a connection to a thermal viewer or video output module*/
void videoConnect() {
	//Disable interrupts
	detachInterrupts();
	//Save old color scheme selection
	byte colorSchemeOld = colorScheme;
	//Allocate space
	rawValues = (unsigned short*)calloc(4800, sizeof(unsigned short));
	//Show message
	drawMessage((char*)"Waiting for connection..");
	display.print((char*) "Touch screen to return", CENTER, 170);
	delay(1000);
	//Wait for device
	while (true) {
		//Serial start command send
		if ((Serial.available() > 0) && (Serial.read() == CMD_START)) {
			//Wait for next byte
			while (Serial.available() == 0) {
				if (touch.touched())
					break;
			}
			//Next byte should be the mode
			byte readMode = Serial.read();
			//We detected the thermal viewer, get the current time
			if (readMode == CMD_THERMALVIEWER) {
				//Wait for next byte
				while (Serial.available() == 0) {
					if (touch.touched())
						break;
				}
				//If we received the time
				if (Serial.available() != 0) {
					String dateIn = Serial.readString();
					//Check if valid
					if (getInt(dateIn.substring(0, 4) >= 2016)) {
						//Set the clock
						setTime(getInt(dateIn.substring(11, 13)), getInt(dateIn.substring(14, 16)), getInt(dateIn.substring(17, 19)), getInt(dateIn.substring(8, 10)), getInt(dateIn.substring(5, 7)), getInt(dateIn.substring(0, 4)));
						//Set the RTC
						Teensy3Clock.set(now());
					}
					drawMessage((char*)"Detected Thermal Viewer!");
					display.print((char*) "Touch screen to return", CENTER, 170);
					delay(1000);
					videoOutType = videoOut_thermalViewer;
				}
				//Otherwise return
				else
					break;
			}
			//We detected the video output module
			else if (readMode == CMD_VIDEOMODULE) {
				drawMessage((char*)"Detected Video Output Module!");
				display.print((char*) "Touch screen to return", CENTER, 170);
				delay(1000);
				videoOutType = videoOut_externalModule;
			}
				
			//We detected the thermo vision application
			else if (readMode == CMD_THERMOVISION) {
				drawMessage((char*)"Detected ThermoVision Software!");
				display.print((char*) "Touch screen to return", CENTER, 170);
				delay(1000);
				videoOutType = videoOut_thermoVision;
			}
			//We detected, that a user wants to get data over the terminal
			else if (readMode == CMD_TERMINAL_CONNECT) {
				drawMessage((char*)"Detected Serial Terminal Client!");
				display.print((char*) "Touch screen to return", CENTER, 170);
				delay(1000);
				videoOutType = videoOut_serialTerminal;
			}
			//None of the options, return
			else
				break;
			//Clear input buffer
			serialClear();
			//Send start byte if not in terminal mode
			if(videoOutType != videoOut_serialTerminal)
				Serial.write(START_SESSION);
			//Start video output
			videoOutput();
			break;
		}
		//Another command received, discard it
		else if ((Serial.available() > 0))
			Serial.read();
		//Abort search
		if (touch.touched())
			break;
	}
	//Show message
	drawMessage((char*)"Connection ended, return..");
	delay(1000);
	//Clear all serial buffers
	serialClear();
	//Free space again
	free(rawValues);
	//Restore values
	colorScheme = colorSchemeOld;
	videoSave = false;
	showMenu = false;
	//Enable interrupts
	attachInterrupts();
}