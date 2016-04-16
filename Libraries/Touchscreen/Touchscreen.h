#ifndef _Touchscreen_h_
#define _Touchscreen_h_

#include "FT6206_Touchscreen.h"
#include "XPT2046_Touchscreen.h"

class Touchscreen {
public:
	//Resistive Touch Controller
	XPT2046_Touchscreen resTouch;
	//Capacitive Touch Controller
	FT6206_Touchscreen capTouch;
	//Choose the right touch screen
	bool capacitive = false;

	/* Check if the capacitive touch can be started, otherwise use resistive */
	void begin() {
		//Capacitive screen
		if (capTouch.begin()) 
			capacitive = true;
		//Resistive screen or none
		else {
			resTouch.begin();
		}
	}

	/* Returns if the screen is currently touched */
	bool touched() {
		if (capacitive)
			return capTouch.touched();
		else
			return resTouch.touched();
	}

	/* Set rotation for touch screen */
	void setRotation(bool rotated) {
		if (capacitive)
			capTouch.rotated = rotated;
		else
			resTouch.rotated = rotated;
	}

	/* Returns the coordinates of the touched point */
	TS_Point getPoint() {
		if (capacitive)
			return capTouch.getPoint();
		else
			return resTouch.getPoint();
	}
};

#endif