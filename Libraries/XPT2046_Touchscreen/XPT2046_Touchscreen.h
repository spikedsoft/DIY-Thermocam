#ifndef _XPT2046_Touchscreen_h_
#define _XPT2046_Touchscreen_h_

#include "Arduino.h"
#include <SPI.h>

class TS_Point {
public:
	TS_Point(void) : x(0), y(0), z(0) {}
	TS_Point(int16_t x, int16_t y, int16_t z) : x(x), y(y), z(z) {}
	bool operator==(TS_Point p) { return ((p.x == x) && (p.y == y) && (p.z == z)); }
	bool operator!=(TS_Point p) { return ((p.x != x) || (p.y != y) || (p.z != z)); }
	int16_t x, y, z;
};

class XPT2046_Touchscreen {
public:
	XPT2046_Touchscreen(uint8_t cspin, uint8_t tirq=255);
	bool begin();
	TS_Point getPoint();
	bool touched();
	void readData(uint16_t *x, uint16_t *y, uint8_t *z);
	bool bufferEmpty();
	uint8_t bufferSize() { return 1; }
	bool isrWake;

private:
	void update();
	uint8_t csPin, tirqPin;
	int16_t xraw, yraw, zraw;
	uint32_t msraw;
};

#endif
