/*
* Main Methods to display the Graphical-User-Interface
*/

#include "Bitmaps.h"

/* Draw a message on the screen */
void drawMessage(char* message) {
	display.fillScr(127, 127, 127);
	display.setFont(smallFont);
	display.setBackColor(127, 127, 127);
	display.setColor(VGA_WHITE);
	display.print(message, CENTER, 110);
}

/* Draw a title on the screen */
void drawTitle(char* name) {
	display.fillScr(127, 127, 127);
	display.setFont(bigFont);
	display.setBackColor(127, 127, 127);
	display.setColor(VGA_WHITE);
	display.print(name, CENTER, 25);
	display.setFont(smallFont);
}

/* Shows the hadware diagnostics */
void showDiagnostic() {
	//Display title & background
	display.fillScr(127, 127, 127);
	display.setFont(bigFont);
	display.setBackColor(127, 127, 127);
	display.setColor(VGA_YELLOW);
	display.print((char*) "Self-diagnostic", CENTER, 10);
	//Display hint
	display.setFont(smallFont);
	display.print((char*) "Fix the issues to use the device", CENTER, 220);
	display.setColor(VGA_WHITE);
	//Display hardware module names
	display.print((char*) "Spot sensor  ", 50, 50);
	display.print((char*) "Display      ", 50, 70);
	display.print((char*) "Visual camera", 50, 90);
	display.print((char*) "Touch screen ", 50, 110);
	display.print((char*) "SD card      ", 50, 130);
	display.print((char*) "Battery gauge", 50, 150);
	display.print((char*) "Lepton config", 50, 170);
	display.print((char*) "Lepton data  ", 50, 190);
	//Check spot sensor
	if (checkDiagnostic(diag_spot))
		display.print((char*) "OK    ", 220, 50);
	else {
		display.print((char*) "Failed", 220, 50);
		//We disable the spot sensor, so the user can go on
		spotEnabled = false;
	}
	//Check display
	if (checkDiagnostic(diag_display))
		display.print((char*) "OK    ", 220, 70);
	else
		display.print((char*) "Failed", 220, 70);
	//Check visual camera
	if (checkDiagnostic(diag_camera))
		display.print((char*) "OK    ", 220, 90);
	else {
		display.print((char*) "Failed", 220, 90);
		//We disable the camera, so the user can go on
		visualEnabled = false;
	}
	//Check touch screen
	if (checkDiagnostic(diag_touch))
		display.print((char*) "OK    ", 220, 110);
	else
		display.print((char*) "Failed", 220, 110);
	//Check sd card
	if (checkDiagnostic(diag_sd))
		display.print((char*) "OK    ", 220, 130);
	else
		display.print((char*) "Failed", 220, 130);
	//Check battery gauge
	if (checkDiagnostic(diag_bat))
		display.print((char*) "OK    ", 220, 150);
	else
		display.print((char*) "Failed", 220, 150);
	//Check lepton config
	if (checkDiagnostic(diag_lep_conf))
		display.print((char*) "OK    ", 220, 170);
	else
		display.print((char*) "Failed", 220, 170);
	//Check lepton data
	if (checkDiagnostic(diag_lep_data))
		display.print((char*) "OK    ", 220, 190);
	else
		display.print((char*) "Failed", 220, 190);
}

/* Show the save message on the screen */
void showSaveMessage() {
	//Thermal only
	if (displayMode == displayMode_thermal) {
		if (!convertEnabled)
			showMsg((char*) "Save Thermal Raw..");
		else
			showMsg((char*) "Save Thermal BMP..");
	}
	//Visual only
	else if (displayMode == displayMode_visual) {
		showMsg((char*) "Save Visual JPEG..");
	}
	//Combined
	else if (displayMode == displayMode_combined) {
		showMsg((char*) "Save Combined BMP..");
	}
	//Set marker to create image
	imgSave = imgSave_create;
}

/* Draw a BigFont Text in the center of a menu*/
void drawCenterElement(int element) {
	display.setFont(bigFont);
	display.setColor(VGA_WHITE);
	display.setBackColor(127, 127, 127);
	display.printNumI(element, CENTER, 80, 2, '0');
	display.setFont(smallFont);
}

/* Converts a given float to char array */
void floatToChar(char* buffer, float val) {
	int units = val;
	int hundredths = val * 100;
	hundredths = abs(hundredths % 100);
	sprintf(buffer, "%d.%02d", units, hundredths);
}

/* Draw the Boot screen */
void bootScreen() {
	//Set Fonts
	touchButtons.setTextFont(smallFont);
	display.setFont(smallFont);
	//Draw Screen
	display.fillScr(255, 255, 255);
	display.setFont(bigFont);
	display.setBackColor(255, 255, 255);
	display.setColor(VGA_BLACK);
	display.writeRect4BPP(90, 35, 140, 149, logoBitmap, logoColors);
	display.print((char*) "Booting", CENTER, 194);
	display.setFont(smallFont);
	display.print((char*)"DIY-Thermocam", CENTER, 10);
	display.print((char*)Version, CENTER, 220);
}

/* Display the warmup message on screen*/
void displayWarmup() {
	char buffer[25];
	sprintf(buffer, "Sensor warmup, %2ds left", (int)abs(60 - ((millis() - calTimer) / 1000)));
	display.print(buffer, 45, 200);
}

/* Display battery status in percentage */
void displayBatteryStatus() {
	//Check battery status
	checkBattery();
	//Display it
	if (batPercentage != -1) {
		display.printNumI(batPercentage, 280, 0, 3, ' ');
		display.print((char*) "%", 310, 0);
	}
	else
		display.print((char*) "USB Power", 240, 0);
}

/* Display the date on screen */
void displayDate() {
	//In live mode
	if (display.writeToImage) {
		display.printNumI(day(), 5, 0, 2, '0');
		display.print((char*) ".", 23, 0);
		display.printNumI(month(), 27, 0, 2, '0');
		display.print((char*) ".", 45, 0);
		display.printNumI(year(), 49, 0, 4);
	}
	//In the menu
	else {
		display.printNumI(day(), 5, 0, 2, '0');
		display.print((char*) ".", 20, 0);
		display.printNumI(month(), 27, 0, 2, '0');
		display.print((char*) ".", 42, 0);
		display.printNumI(year(), 49, 0, 4);
	}
}

/* Display free space on screen*/
void displayFreeSpace() {
	//In live mode
	if (display.writeToImage) {
		if (mlx90614Version == mlx90614Version_new)
			display.print(sdInfo, 207, 228);
	}
	//In menu
	else {
		//Old hw generation - Display FW version
		if (mlx90614Version == mlx90614Version_old) {
			display.print((char*)"Firmware ", 220, 228);
			display.printNumI(fwVersion, 290, 228);
		}
		//All other - Display free space on internal storage
		if (mlx90614Version == mlx90614Version_new)
			display.print(sdInfo, 230, 228);
	}
}

/* Display the current time on the screen*/
void displayTime() {
	//In live mode
	if (display.writeToImage) {
		display.printNumI(hour(), 5, 228, 2, '0');
		display.print((char*) ":", 23, 228);
		display.printNumI(minute(), 27, 228, 2, '0');
		display.print((char*) ":", 45, 228);
		display.printNumI(second(), 49, 228, 2, '0');
	}
	//In the menu
	else {
		display.printNumI(hour(), 5, 228, 2, '0');
		display.print((char*) ":", 20, 228);
		display.printNumI(minute(), 27, 228, 2, '0');
		display.print((char*) ":", 42, 228);
		display.printNumI(second(), 49, 228, 2, '0');
	}
}

/* Updates the additional information on the screen */
void updateInfos(bool refresh) {
	//Set Text Color
	display.setColor(VGA_WHITE);
	display.setBackColor(127, 127, 127);
	if (refresh) {
		//Display battery status on screen
		displayBatteryStatus();
		//Display date
		displayDate();
		//Display free space
		displayFreeSpace();
	}
	//Display time
	displayTime();
}

/* Touch handler for the main menu */
void mainMenuHandler() {
	//Update infos on the screen
	updateInfos(false);
	//Touch pressed
	if (touch.touched() == true) {
		int pressedButton = touchButtons.checkButtons();
		//Live Mode
		if (pressedButton == 0) {
			drawMessage((char*) "Please wait..");
			liveMode();
		}
		//Load Menu
		else if (pressedButton == 1) {
			loadThermal();
		}
		//Settings Menu
		else if (pressedButton == 2) {
			settingsMenu();
			settingsMenuHandler();
		}
		//File Transfer
		else if (pressedButton == 3) {
			massStorage();
		}
	}
}

/* Displays the main menu on the screen */
void mainMenu() {
	drawTitle((char*) "Main Menu");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 130, 70, (char*)"Live Mode");
	touchButtons.addButton(170, 60, 130, 70, (char*) "Load Menu");
	touchButtons.addButton(20, 150, 130, 70,
		(char*) "Settings Menu");
	touchButtons.addButton(170, 150, 130, 70, (char*) "File Transfer");
	touchButtons.drawButtons();
	refreshFreeSpace();
	updateInfos(true);
}

/*Include section */
#include "LiveMenu.h"
#include "LoadMenu.h"
#include "SettingsMenu.h"
#include "FirstStart.h"
#include "VideoMenu.h"
