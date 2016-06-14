/*
* Connection
*/

/* Defines */

#define CMD_VERSION 86
#define CMD_VIDEOMODULE 85
#define CMD_THERMALVIEWER 84
#define CMD_START 83
#define CMD_ROTATED 82
#define CMD_GET 71
#define CMD_END 69
#define CMD_COLORSCHEME 67

#define START_SESSION 0
#define CAP_IMG 100
#define START_VID 150
#define STOP_VID 200
#define SEND_FRAME 250

/* Variables */

//Command, default send frame
byte sendCmd = SEND_FRAME;
//Video out type - 0 = ThermalViewer, 1 = Video out module
bool videoOutType;


/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

unsigned long processSyncMessage() {
	unsigned long pctime = 0L;
	const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013 

	if (Serial.find((char*)TIME_HEADER)) {
		pctime = Serial.parseInt();
		return pctime;
		if (pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
			pctime = 0L; // return 0 to indicate that the time is not valid
		}
	}
	return pctime;
}

/* Mass storage - jump to end of hex file */

// default flag is EEPROM Address 0x00 set to 0xAE
// another (untested) possibility is setting a byte in an uninitialized section of RAM (e.g. DMAMEM)
#define IS_JUMP_TO_OFFSET_FLAG_SET()  (eeprom_read_byte(0x00) == 0xAE)
#define CLEAR_JUMP_FLAG()             eeprom_write_byte(0x00, 0)
#define SET_JUMP_FLAG()               eeprom_write_byte(0x00, 0xAE)

// the assembly code hook must be run inside a C, not C++ function
#ifdef __cplusplus
extern "C" {
#endif
	void jumpToApplicationAt0x38980() {
		/* Load stack pointer and program counter from start of new program */
		asm("movw r0, #0x8880");
		asm("movt r0, #0x0003");
		asm("ldr sp, [r0]");
		asm("ldr pc, [r0, #4]");
	}

	// TODO: set more peripherals back to reset conditions
	void resetPeripherals() {
		/* set (some of) USB back to normal */
		NVIC_DISABLE_IRQ(IRQ_USBOTG);
		NVIC_CLEAR_PENDING(IRQ_USBOTG);
		SIM_SCGC4 &= ~(SIM_SCGC4_USBOTG);

		/* disable all GPIO interrupts */
		NVIC_DISABLE_IRQ(IRQ_PORTA);
		NVIC_DISABLE_IRQ(IRQ_PORTB);
		NVIC_DISABLE_IRQ(IRQ_PORTC);
		NVIC_DISABLE_IRQ(IRQ_PORTD);
		NVIC_DISABLE_IRQ(IRQ_PORTE);

		/* set (some of) ADC1 back to normal */
		// wait until calibration is complete
		while (ADC1_SC3 & ADC_SC3_CAL)
			;

		// clear flag if calibration failed
		if (ADC1_SC3 & 1 << 6)
			ADC1_SC3 |= 1 << 6;

		// clear conversion complete flag (which could trigger ISR otherwise)
		if (ADC1_SC1A & 1 << 7)
			ADC1_SC1A |= 1 << 7;

		/* set some clocks back to default/reset settings */
		MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(4);
		SIM_CLKDIV1 = 0;
		SIM_CLKDIV2 = 0;
	}

	void startup_late_hook(void) {
		// look for the condition that indicates we want to jump to the application with offset
		if (IS_JUMP_TO_OFFSET_FLAG_SET()) {

			// clear the condition
			CLEAR_JUMP_FLAG();

			// set peripherals (mostly) back to normal then jump
			__disable_irq();
			resetPeripherals();
			jumpToApplicationAt0x38980();
		}
	}
#ifdef __cplusplus
}
#endif

#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART() (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

void restartAndJumpToApp(void) {
	SET_JUMP_FLAG();
	CPU_RESTART()
		;
}

/* Methods */

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

/* Evaluates commands from the serial port*/
bool serialHandler() {
	byte recCmd = Serial.read();
	switch (recCmd) {
		//G - Get command
	case CMD_GET:
		//Send command
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
			//Make sure all data has been send
			Serial.flush();
		}
		//Switch back to send frame the next time
		else
			sendCmd = SEND_FRAME;
		break;
		//E - End connection
	case CMD_END:
		return true;
		break;
		//C - Change color scheme
	case CMD_COLORSCHEME:
		if (colorScheme < (colorSchemeTotal - 1))
			colorScheme++;
		else if (colorScheme == (colorSchemeTotal - 1))
			colorScheme = 0;
		break;
		//R - Get rotation
	case CMD_ROTATED:
		Serial.write(rotationEnabled);
		Serial.flush();
		break;
		//V - Send lepton version
	case CMD_VERSION:
		Serial.write(leptonVersion);
		Serial.flush();
		break;
	}
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
	//Show message
	drawMessage((char*) "Connection established!");
	display.print((char*) "Touch screen to return", CENTER, 170);
	delay(1000);
	//Disable screen backlight
	disableScreenLight();
	//Send the frames
	while (true) {
		//Abort transmission
		if (touch.touched())
			break;
		//Get the temps
		getTemperatures(true);
		//For 160x120 Lepton3
		if (leptonVersion == leptonVersion_3_Shutter) {
			//Find min and max if required
			if ((agcEnabled) && (!limitsLocked) && (colorScheme != colorScheme_coldest) && (colorScheme != colorScheme_hottest))
				limitValues();
			//Convert to colors for video out module
			if (videoOutType == 1) {
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
			if(videoOutType == 1) {
				scaleValues(true);
				convertColors(true);
			}
		}
		//Refresh the temp points if enabled
		if (pointsEnabled)
			refreshTempPoints(true);
		//Activate the calibration after warmup
		if (calStatus == cal_warmup) {
			if (millis() - calTimer > 60000) {
				//Perform FFC if shutter is attached
				if (leptonVersion != leptonVersion_2_NoShutter)
					leptonRunCalibration();
				calStatus = cal_standard;
			}
		}
		//Check buttton press
		if (extButtonPressed())
			buttonHandler();
		//Serial command check
		if (Serial.available() > 0) {
			if (serialHandler())
				break;
		}
	}
	//Enable display backlight
	enableScreenLight();
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
			if (readMode == CMD_THERMALVIEWER)
				videoOutType = 0;
			//We detected the video output module
			else if (readMode == CMD_VIDEOMODULE)
				videoOutType = 1;
			//None of the both, return
			else
				break;
			//Clear input buffer
			serialClear();
			//Send start byte
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

/* Go into mass storage mode */
void massStorage() {
	//Old hardware
	if (mlx90614Version == mlx90614Version_old) {
		//Display error msg for 1sec
		drawMessage((char*) "Your HW does not support this!");
		delay(1000);
		//Go back
		mainMenu();
	}
	//Other
	else {
		drawMessage((char*) "Disconnect USB cable to return !");
		delay(1500);
		//No warmup needed after restart if done previously
		if (calStatus != cal_warmup)
			EEPROM.write(eeprom_massStorage, eeprom_setValue);
		//Fix display diagnostic error
		else
			EEPROM.write(eeprom_massStorage, eeprom_massStorage);
		restartAndJumpToApp();
	}
}