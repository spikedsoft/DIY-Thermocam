/*
* Connection Menu
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

void serialInFlush() {
	while (Serial.available() > 0) {
		Serial.read();
	}
}

/* Go into video output mode and wait for connected module */
void videoOutput() {
	byte MSB, LSB;
	drawMessage((char*) "Waiting for thermal viewer..");
	display.print((char*) "Touch screen to return", CENTER, 170);
	//Allocate space
	if (leptonVersion == 0)
		rawValues = (unsigned short*)calloc(4800, sizeof(unsigned short));
	if (leptonVersion == 1)
		image = (unsigned short*)calloc(19200, sizeof(unsigned short));
	while (!Serial.available()) {
		if (touch.touched()) {
			if (leptonVersion == 0)
				free(rawValues);
			if (leptonVersion == 1)
				free(image);
			connectionMenu();
			return;
		}
	}
	serialInFlush();
	drawMessage((char*) "Sending data to the viewer..");
	display.print((char*) "Touch screen to return", CENTER, 170);
	delay(1000);
	//Send config data
	//Serial.write(leptonVersion);
	while (true) {
		if (leptonVersion == 1){
			drawMessage((char*) "Sending data to the viewer..");
			getTemperatures();
		}
		else
			getTemperatures(true);
		if (touch.touched())
			break;
		if (Serial.available() > 0) {
			serialInFlush();
			Serial.clearReadError();
			Serial.clearWriteError();
			if (leptonVersion == 0) {
				for (int i = 0; i < 4800; i++) {
					MSB = (byte)(rawValues[i] & 0xff);
					LSB = (byte)((rawValues[i] >> 8) & 0xff);
					Serial.write(LSB);
					Serial.write(MSB);
				}
			}
			if (leptonVersion == 1) {
				for (int i = 0; i < 19200; i++) {
					MSB = (byte)(image[i] & 0xff);
					LSB = (byte)((image[i] >> 8) & 0xff);
					Serial.write(LSB);
					Serial.write(MSB);
				}
			}
			Serial.flush();
		}
	}
	drawMessage((char*) "Connection ended, return..");
	if (leptonVersion == 0)
		free(rawValues);
	if (leptonVersion == 1)
		free(image);
	delay(1000);
	//Go back
	connectionMenu();
}

/* Go into mass storage mode */
void massStorage() {
	//Early-Bird #1
	if (mlx90614Version == 0) {
		//Display error msg for 1sec
		drawMessage((char*) "Does not work with Early-Bird HW !");
		delay(1000);
		//Go back
		connectionMenu();
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

/* Touch handler for the connection menu */
void connectionMenuHandler() {
	while (1) {
		updateInfos(false);
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Video output
			if (pressedButton == 0) {
				videoOutput();
			}
			//Mass storage
			else if (pressedButton == 1) {
				massStorage();
			}
			//Back
			else if (pressedButton == 2) {
				mainMenu();
				break;
			}
		}
	}
}

/* Connection Menu */
void connectionMenu() {
	drawTitle((char*) "Connection menu");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 130, 70, (char*) "Video output");
	touchButtons.addButton(170, 60, 130, 70, (char*) "Mass storage");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Back");
	touchButtons.drawButtons();
	updateInfos(true);
}