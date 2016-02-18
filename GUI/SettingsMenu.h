/*
* Settings menu to adjust settings
*/

/* Second Menu */
void secondMenu(bool firstStart) {
	drawTitle((char*) "Second");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 70, 70, (char*) "-");
	touchButtons.addButton(230, 60, 70, 70, (char*) "+");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Back");
	touchButtons.drawButtons();
	drawCenterElement(second());
	if (!firstStart)
		updateInfos(true);
	//Touch handler
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//Minus
			if (pressedButton == 0) {
				if (second() >= 0) {
					if (second() > 0)
						setTime(hour(), minute(), second() - 1, day(), month(),
							year());
					else if (second() == 0)
						setTime(hour(), minute(), 59, day(), month(), year());
					drawCenterElement(second());
				}
			}
			//Plus
			else if (pressedButton == 1) {
				if (second() <= 59) {
					if (second() < 59)
						setTime(hour(), minute(), second() + 1, day(), month(),
							year());
					else if (second() == 59)
						setTime(hour(), minute(), 0, day(), month(), year());
					drawCenterElement(second());
				}
			}
			//Back
			else if (pressedButton == 2) {
				Teensy3Clock.set(now());
				timeMenu();
				break;
			}
		}
	}
}

/* Minute Menu */
void minuteMenu(bool firstStart) {
	drawTitle((char*) "Minute");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 70, 70, (char*) "-");
	touchButtons.addButton(230, 60, 70, 70, (char*) "+");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Back");
	touchButtons.drawButtons();
	drawCenterElement(minute());
	if (!firstStart)
		updateInfos(true);
	//Touch handler
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//Minus
			if (pressedButton == 0) {
				if (minute() >= 0) {
					if (minute() > 0)
						setTime(hour(), minute() - 1, second(), day(), month(),
							year());
					else if (minute() == 0)
						setTime(hour(), 59, second(), day(), month(), year());
					drawCenterElement(minute());
				}
			}
			//Plus
			else if (pressedButton == 1) {
				if (minute() <= 59) {
					if (minute() < 59)
						setTime(hour(), minute() + 1, second(), day(), month(),
							year());
					else if (minute() == 59)
						setTime(hour(), 0, second(), day(), month(), year());
					drawCenterElement(minute());
				}
			}
			//Back
			else if (pressedButton == 2) {
				Teensy3Clock.set(now());
				timeMenu();
				break;
			}
		}
	}
}

/* Hour menu */
void hourMenu(bool firstStart) {
	drawTitle((char*) "Hour");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 70, 70, (char*) "-");
	touchButtons.addButton(230, 60, 70, 70, (char*) "+");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Back");
	touchButtons.drawButtons();
	drawCenterElement(hour());
	if (!firstStart)
		updateInfos(true);
	//Touch handler
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//Minus
			if (pressedButton == 0) {
				if (hour() >= 0) {
					if (hour() > 0)
						setTime(hour() - 1, minute(), second(), day(), month(),
							year());
					else if (hour() == 0)
						setTime(23, minute(), second(), day(), month(), year());
					drawCenterElement(hour());
				}
			}
			//Plus
			else if (pressedButton == 1) {
				if (hour() <= 23) {
					if (hour() < 23)
						setTime(hour() + 1, minute(), second(), day(), month(),
							year());
					else if (hour() == 23)
						setTime(0, minute(), second(), day(), month(), year());
					drawCenterElement(hour());
				}
			}
			//Back
			else if (pressedButton == 2) {
				Teensy3Clock.set(now());
				timeMenu();
				break;
			}
		}
	}
}

/* Day Menu */
void dayMenu(bool firstStart) {
	drawTitle((char*) "Day");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 70, 70, (char*) "-");
	touchButtons.addButton(230, 60, 70, 70, (char*) "+");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Back");
	touchButtons.drawButtons();
	drawCenterElement(day());
	if (!firstStart)
		updateInfos(true);
	//Touch handler
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch press
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//Minus
			if (pressedButton == 0) {
				if (day() >= 1) {
					if (day() > 1)
						setTime(hour(), minute(), second(), day() - 1, month(),
							year());
					else if (day() == 1)
						setTime(hour(), minute(), second(), 31, month(),
							year());
					drawCenterElement(day());
				}
			}
			//Plus
			else if (pressedButton == 1) {
				if (day() <= 31) {
					if (day() < 31)
						setTime(hour(), minute(), second(), day() + 1, month(),
							year());
					else if (day() == 31)
						setTime(hour(), minute(), second(), 1, month(), year());
					drawCenterElement(day());
				}
			}
			//Back
			else if (pressedButton == 2) {
				Teensy3Clock.set(now());
				dateMenu();
				break;
			}
		}
	}
}

/* Month Menu */
void monthMenu(bool firstStart) {
	drawTitle((char*) "Month");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 70, 70, (char*) "-");
	touchButtons.addButton(230, 60, 70, 70, (char*) "+");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Back");
	touchButtons.drawButtons();
	drawCenterElement(month());
	if (!firstStart)
		updateInfos(true);
	//Touch handler
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch press
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//Minus
			if (pressedButton == 0) {
				if (month() >= 1) {
					if (month() > 1)
						setTime(hour(), minute(), second(), day(), month() - 1,
							year());
					else if (month() == 1)
						setTime(hour(), minute(), second(), day(), 12, year());
					drawCenterElement(month());
				}
			}
			//Plus
			else if (pressedButton == 1) {
				if (month() <= 12) {
					if (month() < 12)
						setTime(hour(), minute(), second(), day(), month() + 1,
							year());
					else if (month() == 12)
						setTime(hour(), minute(), second(), day(), 1, year());
					drawCenterElement(month());
				}
			}
			//Back
			else if (pressedButton == 2) {
				Teensy3Clock.set(now());
				dateMenu();
				break;
			}
		}
	}
}

/* Year Menu */
void yearMenu(bool firstStart) {
	drawTitle((char*) "Year");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 70, 70, (char*) "-");
	touchButtons.addButton(230, 60, 70, 70, (char*) "+");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Back");
	touchButtons.drawButtons();
	drawCenterElement(year());
	if (!firstStart)
		updateInfos(true);
	//Touch handler
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//Minus
			if (pressedButton == 0) {
				if (year() > 2014) {
					setTime(hour(), minute(), second(), day(), month(),
						year() - 1);
					drawCenterElement(year());
				}
			}
			//Plus
			else if (pressedButton == 1) {
				if (year() < 2099) {
					setTime(hour(), minute(), second(), day(), month(),
						year() + 1);
					drawCenterElement(year());
				}
			}
			//Back
			else if (pressedButton == 2) {
				Teensy3Clock.set(now());
				dateMenu();
				break;
			}
		}
	}
}

/* Date Menu */
void dateMenu(bool firstStart) {
	drawTitle((char*) "Date");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 130, 70, (char*) "Day");
	touchButtons.addButton(170, 60, 130, 70, (char*) "Month");
	touchButtons.addButton(20, 150, 130, 70, (char*) "Year");
	touchButtons.addButton(170, 150, 130, 70, (char*) "Back");
	touchButtons.drawButtons();
	if (!firstStart)
		updateInfos(true);
}

/* Date Menu Handler */
void dateMenuHandler(bool firstStart) {
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Day
			if (pressedButton == 0) {
				dayMenu(firstStart);
			}
			//Month
			else if (pressedButton == 1) {
				monthMenu(firstStart);
			}
			//Year
			else if (pressedButton == 2) {
				yearMenu(firstStart);
			}
			//Back
			else if (pressedButton == 3) {
				timeAndDateMenu(firstStart);
				break;
			}
		}
	}
}

/* Time Menu */
void timeMenu(bool firstStart) {
	drawTitle((char*) "Time");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 130, 70, (char*) "Hour");
	touchButtons.addButton(170, 60, 130, 70, (char*) "Minute");
	touchButtons.addButton(20, 150, 130, 70, (char*) "Second");
	touchButtons.addButton(170, 150, 130, 70, (char*) "Back");
	touchButtons.drawButtons();
	if (!firstStart)
		updateInfos(true);
}

/* Time Menu Handler */
void timeMenuHandler(bool firstStart = false) {
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Hours
			if (pressedButton == 0) {
				hourMenu(firstStart);
			}
			//Minutes
			else if (pressedButton == 1) {
				minuteMenu(firstStart);
			}
			//Seconds
			else if (pressedButton == 2) {
				secondMenu(firstStart);
			}
			//Back
			else if (pressedButton == 3) {
				timeAndDateMenu(firstStart);
				break;
			}
		}
	}
}

/* Time & Date Menu */
void timeAndDateMenu(bool firstStart) {
	drawTitle((char*) "Time & Date");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 130, 70, (char*) "Time");
	touchButtons.addButton(170, 60, 130, 70, (char*) "Date");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Save");
	if (firstStart)
		touchButtons.relabelButton(2, (char*) "Set", false);
	touchButtons.drawButtons();
	if (!firstStart)
		updateInfos(true);
}

/* Time & Date Menu Handler */
void timeAndDateMenuHandler(bool firstStart = false) {
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Time
			if (pressedButton == 0) {
				timeMenu(firstStart);
				timeMenuHandler(firstStart);
			}
			//Date
			else if (pressedButton == 1) {
				dateMenu(firstStart);
				dateMenuHandler(firstStart);
			}
			//Save
			else if (pressedButton == 2) {
				if (firstStart) {
					if (year() < 2015) {
						drawMessage((char*) "Year must be >= 2015 !");
						delay(1000);
						timeAndDateMenu(true);
					}
					else
						break;
				}
				else {
					settingsMenu();
					break;
				}
			}
		}
	}
}

/* Images format menu */
void imagesFormatMenu(bool firstStart = false) {
	drawTitle((char*) "Image Format");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 130, 70, (char*) "Raw Only");
	touchButtons.addButton(170, 60, 130, 70, (char*) "Raw + Bitmap");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Back");
	touchButtons.drawButtons();
	if (imagesFormat == 0)
		touchButtons.setActive(0);
	else
		touchButtons.setActive(1);
	if (!firstStart)
		updateInfos(true);
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Raw Only
			if (pressedButton == 0) {
				if (imagesFormat == 1) {
					imagesFormat = 0;
					touchButtons.setActive(0);
					touchButtons.setInactive(1);
				}
			}
			//Raw + Bitmap
			else if (pressedButton == 1) {
				if (imagesFormat == 0) {
					imagesFormat = 1;
					touchButtons.setActive(1);
					touchButtons.setInactive(0);
				}
			}
			//Back
			else if (pressedButton == 2) {
				//Write new settings to EEPROM
				EEPROM.write(eeprom_imagesFormat, imagesFormat);
				imagesStorageMenu(firstStart);
				break;
			}
		}
	}
}

/* Images type menu */
void imagesTypeMenu(bool firstStart = false) {
	drawTitle((char*) "Image Type");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 120, 70, (char*) "Thermal only");
	touchButtons.addButton(160, 60, 140, 70, (char*) "Thermal + Visual");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Back");
	touchButtons.drawButtons();
	if (imagesType == 0)
		touchButtons.setActive(0);
	else
		touchButtons.setActive(1);
	if (!firstStart)
		updateInfos(true);
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Thermal only
			if (pressedButton == 0) {
				if (imagesType == 1) {
					imagesType = 0;
					touchButtons.setActive(0);
					touchButtons.setInactive(1);
				}
			}
			//Thermal + Visual
			else if (pressedButton == 1) {
				if (imagesType == 0) {
					imagesType = 1;
					touchButtons.setActive(1);
					touchButtons.setInactive(0);
				}
			}
			//Back
			else if (pressedButton == 2) {
				//Write new settings to EEPROM
				EEPROM.write(eeprom_imagesType, imagesType);
				imagesStorageMenu(firstStart);
				break;
			}
		}
	}
}

/* Handler for the Images Storage Menu */
void imagesStorageMenuHandler(bool firstStart) {
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Images Format
			if (pressedButton == 0) {
				imagesFormatMenu();
			}
			//Images Type
			else if (pressedButton == 1) {
				imagesTypeMenu();
			}
			//Save
			else if (pressedButton == 2) {
				if (!firstStart) {
					storageMenu();
				}
				break;
			}
		}
	}
}

/* Images Storage Menu */
void imagesStorageMenu(bool firstStart) {
	drawTitle((char*) "Image Storage");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 130, 70, (char*) "Storage Format");
	touchButtons.addButton(170, 60, 130, 70, (char*) "Storage Type");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Save");
	if (firstStart)
		touchButtons.relabelButton(2, (char*) "Set", false);
	touchButtons.drawButtons();
	if (!firstStart)
		updateInfos(true);
}

/* Videos format menu */
void videosFormatMenu(bool firstStart = false) {
	drawTitle((char*) "Video Format");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 130, 70, (char*) "Raw Only");
	touchButtons.addButton(170, 60, 130, 70, (char*) "Raw + Bitmap");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Back");
	touchButtons.drawButtons();
	if (videosFormat == 0)
		touchButtons.setActive(0);
	else
		touchButtons.setActive(1);
	if (!firstStart)
		updateInfos(true);
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Raw Only
			if (pressedButton == 0) {
				if (videosFormat == 1) {
					videosFormat = 0;
					touchButtons.setActive(0);
					touchButtons.setInactive(1);
				}
			}
			//Raw + Bitmap
			else if (pressedButton == 1) {
				if (videosFormat == 0) {
					videosFormat = 1;
					touchButtons.setActive(1);
					touchButtons.setInactive(0);
				}
			}
			//Back
			else if (pressedButton == 2) {
				//Write new settings to EEPROM
				EEPROM.write(eeprom_videosFormat, videosFormat);
				imagesStorageMenu(firstStart);
				break;
			}
		}
	}
}

/* Videos type menu */
void videosTypeMenu(bool firstStart = false) {
	drawTitle((char*) "Video Type");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 120, 70, (char*) "Thermal only");
	touchButtons.addButton(160, 60, 140, 70, (char*) "Thermal + Visual");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Back");
	touchButtons.drawButtons();
	if (videosType == 0)
		touchButtons.setActive(0);
	else
		touchButtons.setActive(1);
	if (!firstStart)
		updateInfos(true);
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Thermal only
			if (pressedButton == 0) {
				if (videosType == 1) {
					videosType = 0;
					touchButtons.setActive(0);
					touchButtons.setInactive(1);
				}
			}
			//Thermal + Visual
			else if (pressedButton == 1) {
				if (videosType == 0) {
					videosType = 1;
					touchButtons.setActive(1);
					touchButtons.setInactive(0);
				}
			}
			//Back
			else if (pressedButton == 2) {
				//Write new settings to EEPROM
				EEPROM.write(eeprom_videosType, videosType);
				imagesStorageMenu(firstStart);
				break;
			}
		}
	}
}

/* Handler for the Videos Storage Menu */
void videosStorageMenuHandler(bool firstStart) {
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Videos Format
			if (pressedButton == 0) {
				videosFormatMenu();
			}
			//Videos Type
			else if (pressedButton == 1) {
				videosTypeMenu();
			}
			//Save
			else if (pressedButton == 2) {
				if (!firstStart) {
					storageMenu();
				}
				break;
			}
		}
	}
}

/* Videos Storage Menu */
void videosStorageMenu(bool firstStart) {
	drawTitle((char*) "Video Storage");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 130, 70, (char*) "Storage Format");
	touchButtons.addButton(170, 60, 130, 70, (char*) "Storage Type");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Save");
	if (firstStart)
		touchButtons.relabelButton(2, (char*) "Set", false);
	touchButtons.drawButtons();
	if (!firstStart)
		updateInfos(true);
}

/* Asks the user if he really wants to format */
void formatStorage() {
	//Early-Bird #1
	if (mlx90614Version == 0){
		drawMessage((char*) "Checking SD card..");
		if (!checkSDCard()) {
			storageMenu();
			return;
		}
	}
	//Title & Background
	drawTitle((char*) "Storage");
	display.setColor(VGA_WHITE);
	display.setFont(smallFont);
	display.setBackColor(127, 127, 127);
	display.print((char*)"Do you really want to format ?", CENTER, 66);
	display.print((char*)"This will delete all images", CENTER, 105);
	display.print((char*)"and videos on the internal storage.", CENTER, 125);
	//Draw the buttons
	touchButtons.deleteAllButtons();
	touchButtons.setTextFont(bigFont);
	touchButtons.addButton(15, 160, 140, 55, (char*) "No");
	touchButtons.addButton(165, 160, 140, 55, (char*) "Yes");
	touchButtons.drawButtons();
	touchButtons.setTextFont(smallFont);
	updateInfos(true);
	//Touch handler
	while (true) {
		updateInfos(false);
		//If touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons(true);
			//YES
			if (pressedButton == 1) {
				drawMessage((char*) "Formatting storage..");
				formatCard();
				drawMessage((char*) "Format finished !");
				delay(1000);
				refreshFreeSpace();
				break;
			}
			//NO
			else if (pressedButton == 0) {
				break;
			}
		}
	}
	//Go back to the storage menu
	storageMenu();
}

/* Storage menu handler*/
void storageMenuHandler() {
	while (1) {
		updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Images
			if (pressedButton == 0) {
				imagesStorageMenu();
				imagesStorageMenuHandler();
			}
			//Videos
			else if (pressedButton == 1) {
				videosStorageMenu();
				videosStorageMenuHandler();
			}
			//Format
			else if (pressedButton == 2) {
				formatStorage();
			}
			//Back
			else if (pressedButton == 3) {
				settingsMenu();
				break;
			}
		}
	}
}

/* Storage menu */
void storageMenu() {
	drawTitle((char*) "Storage");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 130, 70, (char*) "Image Storage");
	touchButtons.addButton(170, 60, 130, 70, (char*) "Video Storage");
	touchButtons.addButton(20, 150, 130, 70, (char*) "Format");
	touchButtons.addButton(170, 150, 130, 70, (char*) "Back");
	touchButtons.drawButtons();
	updateInfos(true);

}

/* Temperature format menu */
void tempFormatMenu(bool firstStart = false) {
	drawTitle((char*) "Temp. Format");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 130, 70, (char*) "Celcius");
	touchButtons.addButton(170, 60, 130, 70, (char*) "Fahrenheit");
	touchButtons.addButton(20, 150, 280, 70, (char*) "Save");
	if (firstStart)
		touchButtons.relabelButton(2, (char*) "Set", false);
	touchButtons.drawButtons();
	if (tempFormat == 0)
		touchButtons.setActive(0);
	else
		touchButtons.setActive(1);
	if (!firstStart)
		updateInfos(true);
	while (1) {
		if (!firstStart)
			updateInfos(false);
		//touch pressed
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Celcius
			if (pressedButton == 0) {
				if (tempFormat == 1) {
					tempFormat = 0;
					touchButtons.setActive(0);
					touchButtons.setInactive(1);
				}
			}
			//Fahrenheit
			else if (pressedButton == 1) {
				if (tempFormat == 0) {
					tempFormat = 1;
					touchButtons.setActive(1);
					touchButtons.setInactive(0);
				}
			}
			//Save
			else if (pressedButton == 2) {
				//Write new settings to EEPROM
				EEPROM.write(eeprom_tempFormat, tempFormat);
				if (firstStart)
					return;
				else {
					settingsMenu();
				}
				break;
			}
		}
	}
}

/* Touch handler for the settings menu */
void settingsMenuHandler() {
	while (1) {
		updateInfos(false);
		//touch press
		if (touch.touched() == true) {
			int pressedButton = touchButtons.checkButtons();
			//Temp. format
			if (pressedButton == 0) {
				tempFormatMenu();
			}
			//Storage
			else if (pressedButton == 1) {
				storageMenu();
				storageMenuHandler();
			}
			//Time & Date
			else if (pressedButton == 2) {
				timeAndDateMenu();
				timeAndDateMenuHandler();
			}
			//Back
			else if (pressedButton == 3) {
				drawMessage((char*)"Settings have been saved !");
				delay(500);
				mainMenu();
				break;
			}
		}
	}
}

/* Settings menu main screen */
void settingsMenu() {
	drawTitle((char*) "Settings");
	touchButtons.deleteAllButtons();
	touchButtons.addButton(20, 60, 130, 70, (char*) "Temp. Format");
	touchButtons.addButton(170, 60, 130, 70, (char*) "Storage");
	touchButtons.addButton(20, 150, 130, 70, (char*) "Time & Date");
	touchButtons.addButton(170, 150, 130, 70, (char*) "Back");
	touchButtons.drawButtons();
	updateInfos(true);
}