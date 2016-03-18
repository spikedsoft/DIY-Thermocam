#include "Camera.h"

void Camera::common_init(void) {
	hwSerial = NULL;
	frameptr = 0;
	bufferLen = 0;
	serialNum = 0;
}
Camera::Camera(HardwareSerial *ser) {
	common_init();
	hwSerial = ser;
}

boolean Camera::begin(uint32_t baud) {
	hwSerial->begin(38400);
	delay(15);
	if (baud != 38400) {
		changeBaudRate();
		hwSerial->begin(115200);
		delay(15);
		return true;
	}
	return reset();
}

boolean Camera::reset() {
	uint8_t args[] = { 0x0 };
	return runCommand(0x26, args, 1, 5);
}

boolean Camera::end() {
	uint8_t args[] = { 0x01, 0x03 };
	return runCommand(0x36, args, sizeof(args), 5);
}

boolean Camera::changeBaudRate() {
	uint8_t args[] = { 0x03, 0x01, 0x0D, 0xA6 };
	return runCommand(0x24, args, sizeof(args), 5);
}

boolean Camera::setImageSize(uint8_t x) {
	uint8_t args[] = { 0x05, 0x04, 0x01, 0x00, 0x19, x };
	return runCommand(0x31, args, sizeof(args), 5);
}

uint8_t Camera::getImageSize() {
	uint8_t args[] = { 0x4, 0x4, 0x1, 0x00, 0x19 };
	if (!runCommand(0x30, args, sizeof(args), 6))
		return -1;

	return camerabuff[5];
}

boolean Camera::setCompression(uint8_t c) {
	uint8_t args[] = { 0x5, 0x1, 0x1, 0x12, 0x04, c };
	return runCommand(0x31, args, sizeof(args), 5);
}

boolean Camera::takePicture() {
	frameptr = 0;
	return cameraFrameBuffCtrl(0x0);
}

boolean Camera::cameraFrameBuffCtrl(uint8_t command) {
	uint8_t args[] = { 0x1, command };
	return runCommand(0x36, args, sizeof(args), 5);
}

uint32_t Camera::frameLength(void) {
	uint8_t args[] = { 0x01, 0x00 };
	if (!runCommand(0x34, args, sizeof(args), 9))
		return 0;
	uint32_t len;
	len = camerabuff[5];
	len <<= 8;
	len |= camerabuff[6];
	len <<= 8;
	len |= camerabuff[7];
	len <<= 8;
	len |= camerabuff[8];
	return len;
}


uint8_t Camera::available(void) {
	return bufferLen;
}

uint8_t * Camera::readPicture(uint8_t n) {
	uint8_t args[] = { 0x0C, 0x0, 0x0A,
					  0, 0, (uint8_t)(frameptr >> 8), (uint8_t)(frameptr & 0xFF),
					  0, 0, 0, n,
					  10 >> 8, 10 & 0xFF };

	if (!runCommand(0x32, args, sizeof(args), 5, false))
		return 0;
	if (readResponse(n + 5, 10) == 0)
		return 0;
	frameptr += n;
	return camerabuff;
}

boolean Camera::runCommand(uint8_t cmd, uint8_t *args, uint8_t argn,
	uint8_t resplen, boolean flushflag) {
	if (flushflag) {
		readResponse(100, 10);
	}
	sendCommand(cmd, args, argn);
	if (readResponse(resplen, 200) != resplen)
		return false;
	if (!verifyResponse(cmd))
		return false;
	return true;
}

void Camera::sendCommand(uint8_t cmd, uint8_t args[] = 0, uint8_t argn = 0) {
	hwSerial->print(0x56, BYTE);
	hwSerial->print(serialNum, BYTE);
	hwSerial->print(cmd, BYTE);

	for (uint8_t i = 0; i < argn; i++) {
		hwSerial->print(args[i], BYTE);
	}
}

uint8_t Camera::readResponse(uint8_t numbytes, uint8_t timeout) {
	uint8_t counter = 0;
	bufferLen = 0;
	int avail;
	while ((timeout != counter) && (bufferLen != numbytes)) {
		avail = hwSerial->available();
		if (avail <= 0) {
			delay(1);
			counter++;
			continue;
		}
		counter = 0;
		camerabuff[bufferLen++] = hwSerial->read();
	}
	return bufferLen;
}

boolean Camera::verifyResponse(uint8_t command) {
	if ((camerabuff[0] != 0x76) ||
		(camerabuff[1] != serialNum) ||
		(camerabuff[2] != command) ||
		(camerabuff[3] != 0x0))
		return false;
	return true;
}
