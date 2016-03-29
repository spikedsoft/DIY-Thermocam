/*
* Connection
*/

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
	//For the Lepton2 sensor
	if (leptonVersion != 1)
		limitLeptonSaveValues(4800, rawValues);
	//For the Lepton3 sensor
	else
		limitLeptonSaveValues(19200, image);
	//Send min
	Serial.write((minTemp & 0xFF00) >> 8);
	Serial.write(minTemp & 0x00FF);
	//Send max
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
	if (calStatus == 0)
		Serial.write((byte)0);
	else
		Serial.write(colorbarEnabled);
	//Send the temperature points enabled attribute
	if (calStatus == 0)
		Serial.write((byte)0);
	else
		Serial.write(pointsEnabled);
	//Send the calibration offset
	if (calStatus != 2)
		calOffset = mlx90614GetAmb() - 204.8;
	floatToBytes(farray, (float)calOffset);
	for (int i = 0; i < 4; i++)
		Serial.write(farray[i]);
	//Send the calibration slope
	floatToBytes(farray, (float)calSlope);
	for (int i = 0; i < 4; i++)
		Serial.write(farray[i]);
	//Send the temperature points
	for (int i = 0; i < 192; i++) {
		Serial.write((showTemp[i] & 0xFF00) >> 8);
		Serial.write(showTemp[i] & 0x00FF);
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
	if (leptonVersion != 1) {
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

/* Go into video output mode and wait for connected module */
void videoOutput() {
	//Save old color scheme selection
	byte colorSchemeOld = colorScheme;
	//Allocate space
	rawValues = (unsigned short*)calloc(4800, sizeof(unsigned short));
	//Show message
	drawMessage((char*) "Sending data to the viewer..");
	display.print((char*) "Touch screen to return", CENTER, 170);
	delay(1000);
	//Disable screen backlight
	digitalWrite(pin_lcd_backlight, LOW);
	//Send config data
	Serial.write(leptonVersion);
	//Send the frames
	while (true) {
		//Abort transmission
		if (touch.touched())
			break;
		//Get the temps
		getTemperatures(true);
		//Activate the calibration after warmup
		if (calStatus == 0) {
			if (millis() - calTimer > 60000) {
				//Perform FFC if shutter is attached
				if (leptonVersion != 2)
					leptonRunCalibration();
				calStatus = 1;
			}
		}
		//Serial command send
		if (Serial.available() > 0) {
			byte cmd = Serial.read();
			//G - Get frame
			if (cmd == 71) {
				//Clear all serial buffers
				serialClear();
				//Send Lepton data
				sendLeptonData();
				//Send config data
				sendConfigData();
				//Make sure all data has been send
				Serial.flush();
			}
			//E - End connection
			else if (cmd == 69)
				break;
			//C - Change color scheme
			else if (cmd == 67) {
				if (colorScheme < 17)
					colorScheme++;
				else if (colorScheme == 17)
					colorScheme = 0;
			}
		}
	}
	//Enable display backlight
	digitalWrite(pin_lcd_backlight, HIGH);
	//Clear all serial buffers
	serialClear();
	//Free space again
	free(rawValues);
	//Show return message
	drawMessage((char*) "Connection ended, return..");
	delay(1000);
	//Restore values
	colorScheme = colorSchemeOld;
	showMenu = false;
}

/* Go into mass storage mode */
void massStorage() {
	//Early-Bird #1
	if (mlx90614Version == 0) {
		//Display error msg for 1sec
		drawMessage((char*) "Does not work with Early-Bird HW !");
		delay(1000);
		//Go back
		mainMenu();
	}
	//Other
	else {
		drawMessage((char*) "Disconnect USB cable to return !");
		delay(1500);
		//No warmup needed after restart if done previously
		if (calStatus != 0)
			EEPROM.write(eeprom_massStorage, eeprom_setValue);
		restartAndJumpToApp();
	}
}