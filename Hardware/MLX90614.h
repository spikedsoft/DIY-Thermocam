/*
* Access the MLX90614 single-point IR sensor
*/

/* Variables */

//CRC Table to calculate I2C PEC
const unsigned char mlx90614CrcTable[] = { 0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15, 0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D, 0x70, 0x77, 0x7E,
0x79, 0x6C, 0x6B, 0x62, 0x65, 0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D, 0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5, 0xD8, 0xDF,
0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD, 0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85, 0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD, 0xC7,
0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2, 0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA, 0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A, 0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32, 0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D,
0x0A, 0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42, 0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A, 0x89, 0x8E, 0x87, 0x80, 0x95, 0x92,
0x9B, 0x9C, 0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4, 0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC, 0xC1, 0xC6, 0xCF, 0xC8, 0xDD,
0xDA, 0xD3, 0xD4, 0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C, 0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44, 0x19, 0x1E, 0x17, 0x10,
0x05, 0x02, 0x0B, 0x0C, 0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34, 0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B, 0x76, 0x71, 0x78,
0x7F, 0x6A, 0x6D, 0x64, 0x63, 0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B, 0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13, 0xAE, 0xA9,
0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB, 0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83, 0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB, 0xE6,
0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3 };
//Stores the object temp
float mlx90614Temp;
//Stores the ambient temp
float mlx90614Amb;

/* Methods */

/* Calculate CRC8 checksum for PEC */
char mlx90614CRC8(byte buffer[], int len) {
	unsigned char m_crc = 0;
	int m_byte_count = 0;
	for (int i = 0; i < len; i++) {
		m_byte_count++;
		m_crc = mlx90614CrcTable[m_crc ^ buffer[i]];
	}
	return m_crc;
}

/* Receive data from the RAM over I2C */
uint16_t mlx90614GetRawData(bool TaTo, bool* check) {
	// Store the two relevant bytes of data for temperature
	byte dataLow = 0x00;
	byte dataHigh = 0x00;
	Wire.beginTransmission(0x5A);
	if (TaTo)
		Wire.send(0x06); //measure ambient temp
	else {
		Wire.send(0x07); // measure objec temp
	}
	Wire.endTransmission(I2C_NOSTOP);
	Wire.requestFrom(0x5A, 3);
	if (Wire.available() != 3)
		*check = false;
	dataLow = Wire.read();
	if (dataLow == -1)
		*check = false;
	dataHigh = Wire.read();
	if (dataHigh == -1)
		*check = false;
	byte pec = Wire.read();
	if (pec == -1)
		*check = false;
	Wire.endTransmission();
	uint16_t tempData = (((dataHigh & 0x007F) << 8) + dataLow);
	return tempData;
}

/* Send data to the EEPROM over I2C*/
void mlx90614Send(byte address, byte LSB, byte MSB) {
	Wire.beginTransmission(0x00);
	Wire.write(address);
	Wire.write(LSB);
	Wire.write(MSB);
	byte msg[4] = { 0x00, address, LSB, MSB };
	byte PEC = mlx90614CRC8(msg, 4);
	Wire.write(PEC);
	Wire.endTransmission();
	delay(10);
}

/* Receive data from the EEPROM over I2C */
uint16_t mlx90614Receive(byte address) {
	Wire.beginTransmission(0x00);
	Wire.write(address);
	Wire.endTransmission(I2C_NOSTOP);
	Wire.requestFrom(0x00, 3);
	byte LSB = Wire.read();
	byte MSB = Wire.read();
	Wire.read();
	Wire.endTransmission();
	delay(10);
	uint16_t regValue = (((MSB) << 8) + LSB);
	return regValue;
}

/* Measures the ambient or object temperature */
void mlx90614Measure(bool TaTo, bool* check) {
	uint16_t rawData = mlx90614GetRawData(TaTo, check);
	float tempData = (rawData * 0.02) - 0.01;
	tempData -= 273.15;
	//TaTo is one, measure ambient
	if (TaTo) {
		if ((tempData < -40) || (tempData > 125))
			*check = false;
		else
			mlx90614Amb = tempData;
	}
	//TaTo is zero, measure object
	else {
		if ((tempData < -70) || (tempData > 380))
			*check = false;
		else
			mlx90614Temp = tempData;
	}
}

/* Check if the maximum temp is 380°C */
void mlx90614CheckMaxTemp() {
	//Check if maximum temp setting is correct
	while (mlx90614Receive(0x20) != 65315) {
		//If it is not, set it to zero first
		mlx90614Send(0x20, 0x00, 0x00);
		//Then write the new value
		mlx90614Send(0x20, 0x23, 0xFF);
		delay(20);
	}
}

/* Check if the minimum tmep is -70°C */
void mlx90614CheckMinTemp() {
	//Check if minimum temp setting is correct
	while (mlx90614Receive(0x21) != 20315) {
		//If it is not, set it to zero first
		mlx90614Send(0x21, 0x00, 0x00);
		//Then write the new value
		mlx90614Send(0x21, 0x5B, 0x4F);
		delay(20);
	}
}

/* Check if the filter settings match for noise-reduction (50% IIR, max settling time) */
bool mlx90614CheckFilterTemp() {
	//Check if the filter settings for the BCI model are correct
	bool newSettings = false;
	uint16_t filter = mlx90614Receive(0x25);
	mlx90614Version = (filter >> 13) & 1;
	uint16_t filterSettings;
	byte MSB, LSB;
	//MLX90614-BCI with gain factor of 12.5
	if (mlx90614Version == 0) {
		filterSettings = 40816;
		if (filter != filterSettings)
			newSettings = true;
		MSB = 0x70;
		LSB = 0x9F;
	}
	//MLX90614-DCI or MLX90614-DCH with gain factor of 100
	else if (mlx90614Version == 1) {
		filterSettings = 46960;
		if (filter != filterSettings)
			newSettings = true;
		MSB = 0x70;
		LSB = 0xB7;
	}
	//Write new filter settings
	if (newSettings) {
		while (mlx90614Receive(0x25) != filterSettings) {
			//If it is not, set it to zero first
			mlx90614Send(0x25, 0x00, 0x00);
			//Then write the new value
			mlx90614Send(0x25, MSB, LSB);
			delay(20);
		}
		return true;
	}
	//No new settings needed
	else
		return false;
}

/* Read and return the ambient temperature */
float mlx90614GetAmb() {
	bool check;
	do {
		check = true;
		mlx90614Measure(1, &check);
	} while (check == false);
	return mlx90614Amb;
}

/* Read and return the object temperature */
float mlx90614GetTemp() {
	bool check;
	do {
		check = true;
		mlx90614Measure(0, &check);
	} while (check == false);
	return mlx90614Temp;
}

/* Get the emissivity coefficient */
float mlx90614GetEmissivity() {
	uint16_t value = mlx90614Receive(0x24);
	float emissivity = value / 65535.0;
	emissivity = ((int)((emissivity * 100) + 0.5)) / 100.0;
	return emissivity;
}

/*Set a new emissivity coefficient */
bool mlx90614SetEmissivity(float emissivity) {
	if ((emissivity >= 0.01) && (emissivity <= 1.0)) {
		int value = ((int)(65535.0 * emissivity));
		byte lsb = (byte)(value & 0xFF);
		byte msb = (byte)((value >> 8) & 0xFF);
		bool check;
		check = true;
		mlx90614Send(0x24, 0x00, 0x00);
		delay(100);
		mlx90614Send(0x24, lsb, msb);
		delay(100);
		for (int i = 0; i < 10; i++) {
			char buf1[10];
			char buf2[10];
			floatToChar(buf1, mlx90614GetEmissivity());
			floatToChar(buf2, emissivity);
			if (strcmp(buf1, buf2) != 0)
				check = false;
			mlx90614Measure(0, &check);
			delay(10);
		}
		if (check)
			return true;
		else
			return false;
	}
	return false;
}

/* Initializes the sensor */
void mlx90614Init() {
	//Check if the MLX90614 is connected
	bool check = true;
	byte count = 0;
	do {
		mlx90614Measure(0, &check);
		if (count == 100) {
			drawMessage((char*) "Error connecting to IR sensor!");
			while (1);
		}
		count++;
		delay(10);
	} while (!check);
	mlx90614Temp = 0;
	mlx90614Amb = 0;
	//Set maximum temp to 380°C if not set
	mlx90614CheckMaxTemp();
	//Set minimum temp to -70°C if not set
	mlx90614CheckMinTemp();
	//Set emissivity to 0.90 if no already set
	if (mlx90614GetEmissivity() != 0.90)
		mlx90614SetEmissivity(0.90);
	//Set filter settings
	if (mlx90614CheckFilterTemp() && (EEPROM.read(eeprom_firstStart) == eeprom_setValue)) {
		drawMessage((char*) "Please reboot the device manually.");
		display.print((char*) "New filter settings flashed !", CENTER, 80);
		while (true);
	}
}