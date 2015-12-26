/*
* Main Methods to display the Graphical-User-Interface
*/

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

/* Draw a BigFont Text in the center of a menu*/
void drawCenterElement(int element) {
	display.setFont(bigFont);
	display.setColor(VGA_WHITE);
	display.setBackColor(127, 127, 127);
	display.printNumI(element, CENTER, 80, 2, '0');
	display.setFont(smallFont);
}

/* Sets the color according to the selected color scheme */
void setColor() {
	//Rainbow, display white text
	if (colorScheme == 0)
		display.setColor(VGA_WHITE);
	//Ironblack, display black text
	else if (colorScheme == 1)
		display.setColor(VGA_BLUE);
	//For hot & cold
	else
		display.setColor(VGA_GREEN);
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
	display.fillScr(127, 127, 127);
	display.setFont(bigFont);
	display.setBackColor(127, 127, 127);
	display.setColor(VGA_WHITE);
	display.print((char*) "Booting..", CENTER, 110);
	display.setFont(smallFont);
	display.print((char*)"DIY-Thermocam", CENTER, 10);
	display.print((char*)Version, CENTER, 220);
}

/* Updates the additional information on the screen */
void updateInfos(bool refresh) {
	//Set Text Color
	display.setColor(VGA_WHITE);
	display.setBackColor(127, 127, 127);
	//Update battery stats and every 60s
	if ((millis() - batRefreshTime) > 60000)
		checkBattery();
	if(refresh){
		//Display battery status on screen
		drawBatteryStat();
		//Display date
		display.printNumI(day(), 5, 0, 2, '0');
		display.print((char*) ".", 20, 0);
		display.printNumI(month(), 27, 0, 2, '0');
		display.print((char*) ".", 42, 0);
		display.printNumI(year(), 49, 0, 4);
		//Early-Bird #1 - Display Version
		if(mlx90614Version == 0)
			display.print((char*) Version, 200, 228);
		//Early-Bird #2 or Batch #1 - Display free space on internal storage
		else
			display.print(sdInfo, 220, 228);
	}
	//Display time
	display.printNumI(hour(), 5, 228, 2, '0');
	display.print((char*) ":", 20, 228);
	display.printNumI(minute(), 27, 228, 2, '0');
	display.print((char*) ":", 42, 228);
	display.printNumI(second(), 49, 228, 2, '0');
}

/* Touch handler for the main menu */
void mainMenuHandler() {
	//Update infos on the screen
	updateInfos(false);
	//Touch pressed
	if (touch.touched() == true) {
		int pressedButton = touchButtons.checkButtons();
		//Create Image
		if (pressedButton == 0) {
			drawMessage((char*) "Please wait..");
			liveMode();
		}
		//Open Image/Video
		else if (pressedButton == 1) {
			loadThermal();
		}
		//Settings
		else if (pressedButton == 2) {
			settingsMenu();
			settingsMenuHandler();
		}
		//About
		else if (pressedButton == 3) {
			connectionMenu();
			connectionMenuHandler();
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
	touchButtons.addButton(170, 150, 130, 70, (char*) "Connection Menu");
	touchButtons.drawButtons();
	refreshFreeSpace();
	updateInfos(true);
}

/*Include section */
#include "ConMenu.h"
#include "LiveMenu.h"
#include "LoadMenu.h"
#include "SettingsMenu.h"
#include "FirstStart.h"
#include "VideoMenu.h"
