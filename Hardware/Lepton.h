/*
* Access the FLIR Lepton LWIR module
*/

/* Variables */
//Array to store one Lepton frame
byte leptonFrame[164];

/* Methods */

/* Start Lepton SPI Transmission */
void leptonBeginSPI() {
	int clockspeed;
	//Lepton3 - 30 Mhz minimum
	if (leptonVersion == 1)
		clockspeed = 30000000;
	//Lepton2 - 20 Mhz maximum
	else
		clockspeed = 20000000;
	//Start alternative clock line expcept for Early-Bird #1
	if (mlx90614Version == 1)
		startAltClockline();
	//Begin SPI Transaction on alternative Clock
	SPI.beginTransaction(SPISettings(clockspeed, MSBFIRST, SPI_MODE1));
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

/* Trigger a RAD FFC on the Lepton */
void leptonRunFFC() {
	byte error;
	byte errorCounter = 0;
	bool showError = false;
	do {
		Wire.beginTransmission(0x2A);
		Wire.write(0x00);
		Wire.write(0x04);
		Wire.write(0x4E);
		Wire.write(0x2E);
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

/* Set the shutter operation to manual */
void leptonSetFFCManual()
{
	//Read command
	Wire.beginTransmission(0x2A);
	Wire.write(0x00);
	Wire.write(0x04);
	Wire.write(0x02);
	Wire.write(((0x3C) & 0xfc) | ((0x3C >> 2) & 0x3));
	//Read old FFC package first
	while (leptonReadReg(0x2) & 0x01);
	int payload_length = leptonReadReg(0x6);
	Wire.requestFrom(0x2A, payload_length);
	byte package[32];
	for (byte i = 0; i < payload_length; i++)
	{
		package[i] = Wire.read();
	}
	//Alter the second bit to set FFC to manual
	package[1] = 0x00;
	//Transmit the new package
	Wire.beginTransmission(0x2A);
	Wire.write(0x00);
	Wire.write(0x08);
	for (int i = 0; i < 32; i++) {
		Wire.write(package[i]);
	}
	Wire.endTransmission();
	//Package length, use 4 here
	Wire.beginTransmission(0x2A);
	Wire.write(0x00);
	Wire.write(0x06);
	Wire.write(0x00);
	Wire.write(0x04);
	Wire.endTransmission();
	//Module and command ID
	Wire.beginTransmission(0x2A);
	Wire.write(0x00);
	Wire.write(0x04);
	Wire.write(0x02);
	Wire.write(0x3D);
	Wire.endTransmission();
}

/* Set the Radiometry Mode over I2C */
void leptonRadSet(bool enable)
{
	//Enable or disable radiometry
	Wire.beginTransmission(0x2A);
	Wire.write(0x00);
	Wire.write(0x08);
	Wire.write(0x00);
	Wire.write(enable);
	Wire.endTransmission();
	//Data length
	Wire.beginTransmission(0x2A);
	Wire.write(0x00);
	Wire.write(0x06);
	Wire.write(0x00);
	Wire.write(0x02);
	Wire.endTransmission();
	//RAD module with OEM bit and command
	Wire.beginTransmission(0x2A);
	Wire.write(0x00);
	Wire.write(0x04);
	Wire.write(0x4E);
	Wire.write(0x11);
	Wire.endTransmission();
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
	//Short delay to ensure FFC is performed
	delay(1500);
	//Check the Lepton HW Revision
	leptonCheckVersion();
	//Set Lepton FFC mode to manual
	leptonSetFFCManual();
	//Activate Radiometry mode on the Lepton
	leptonRadSet(true);
	//Run the RAD FFC if a shutter is attached
	if (leptonVersion != 2)
		leptonRunFFC();
	//Do a quick calibration
	quickCalibration();
}