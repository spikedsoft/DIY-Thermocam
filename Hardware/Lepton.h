/*
* Access the FLIR Lepton LWIR module
*/

/* Variables */
//Array to store one Lepton frame
byte leptonFrame[164];

/* Methods */

/* Start Lepton SPI Transmission */
void leptonBeginSPI() {
	//Lepton3 - 30 Mhz minimum and SPI mode 0
	if (leptonVersion == 1)
		SPI.beginTransaction(SPISettings(30000000, MSBFIRST, SPI_MODE0));
	//Lepton2 - 20 Mhz maximum and SPI mode 1
	else
		SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE1));
	//Start alternative clock line expcept for Early-Bird #1
	if (mlx90614Version == 1)
		startAltClockline();
	//Start transfer  - CS LOW
	digitalWriteFast(15, LOW);
}

/* End Lepton SPI Transmission */
void leptonEndSPI() {
	//End transfer - CS HIGH
	digitalWriteFast(15, HIGH);
	//End SPI Transaction
	SPI.endTransaction();
	//End alternative clock line except for Early-Bird #2
	if (mlx90614Version == 1)
		endAltClockline();
}

/* Reads one line (164 Bytes) from the lepton over SPI */
boolean leptonReadFrame(uint8_t line, uint8_t seg) {
	bool success = true;
	//Receive one frame over SPI
	do {
		SPI.transfer(leptonFrame, 164);
	}
	//Repeat as long as the frame is not valid, equals sync
	while ((leptonFrame[0] & 0x0F) == 0x0F);
	//Check if the line number matches the expected line
	if (leptonFrame[1] != line) {
		success = false;
	}
	//For the Lepton3, check if the segment number matche
	if ((line == 20) && (leptonVersion == 1)) {
		byte segment = (leptonFrame[0] >> 4);
		if (segment != seg) {
			success = false;
		}
	}
	return success;
}

/* Trigger a flat-field-correction on the Lepton */
void leptonRunCalibration() {
	byte error;
	byte errorCounter = 0;
	bool showError = false;
	do {
		Wire.beginTransmission(0x2A);
		Wire.write(0x00);
		Wire.write(0x04);
		Wire.write(0x02);
		Wire.write(0x42);
		error = Wire.endTransmission();
		if (error) {
			errorCounter++;
			delay(10);
		}
		//Show an error message on screen
		if ((error) && (!showError) && (errorCounter > 10)) {
			showError = true;
			drawMessage((char*) "Please check Lepton I2C connection !");
		}
	} while (error != 0);
	//Wait some time
	delay(2000);
}

/* Select I2C Register on the Lepton */
void leptonSetReg(byte reg) {
	Wire.beginTransmission(0x2A);
	Wire.write(reg >> 8 & 0xff);
	Wire.write(reg & 0xff);
	Wire.endTransmission();
}

/* Read I2C Register on the lepton */
int leptonReadReg(byte reg) {
	uint16_t reading = 0;
	leptonSetReg(reg);
	Wire.requestFrom(0x2A, 2);
	reading = Wire.read();
	reading = reading << 8;
	reading |= Wire.read();
	return reading;
}

/* Checks the Lepton hardware revision */
void leptonCheckVersion() {
	//Get AGC Command
	Wire.beginTransmission(0x2A);
	Wire.write(0x00);
	Wire.write(0x04);
	Wire.write(0x48);
	Wire.write(0x1C);
	byte error = Wire.endTransmission();
	if (error != 0) {
		drawMessage((char*) "FLIR Lepton I2C error!");
		while (1);
	}
	while (leptonReadReg(0x2) & 0x01);
	Wire.requestFrom(0x2A, leptonReadReg(0x6));
	char leptonhw[33];
	Wire.readBytes(leptonhw, 32);
	//Detected Lepton2 Shuttered
	if (strstr(leptonhw, "05-060950-") != NULL) {
		leptonVersion = 0;
	}
	//Detected Lepton3 Shuttered
	else if (strstr(leptonhw, "05-070530-") != NULL) {
		leptonVersion = 1;
		//Disable filter
		filterEnabled = false;
	}
	//Detected Lepton2 No-Shutter
	else {
		leptonVersion = 2;
	}
}

/* Check which hardware revision of the FLIR Lepton is connected */
void initLepton() {
	//Short delay
	delay(1500);
	//Check the Lepton HW Revision
	leptonCheckVersion();
	//Perform FFC if shutter is attached
	if (leptonVersion != 2) 
		leptonRunCalibration();
	//Set the calibration timer
	calTimer = millis();
}