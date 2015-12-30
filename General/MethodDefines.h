/*
* Abstract method defines
*/

void drawMessage(char*);
void firstStart();
bool calibrate();
void startAltClockline(bool sdStart = false);
void endAltClockline();
bool tempLimits();
float calFunction(uint16_t rawValue);
void createThermalImg(bool menu = false);
void calibrationProccess();
void floatToChar(char* buffer, float val);
void showSpot(bool save = false);
void showColorBar();
void toggleSpot();
void toggleFilter();
void settingsMenu();
void timeMenu(bool firstStart = false);
void dateMenu(bool firstStart = false);
void limitValues();
void timeAndDateMenu(bool firstStart = false);
void saveRawData(bool image, char* name, uint16_t framesCaptured = 0);
void proccessVideoFrames(uint16_t framesCaptured, char* dirname);
void createVideoFolder(char* dirname);
void buttonIRQ();
void touchIRQ();
void fillImageArray();
void gaussianBlur(unsigned short *img, long width, long height, float sigma,
	int numsteps);
void convertColors();
void scaleValues();
void saveVisualFrame(uint16_t count, char* dirname);
void bootScreen();
void imagesStorageMenu(bool firstStart = false);
void imagesStorageMenuHandler(bool firstStart = false);
void videosStorageMenu(bool firstStart = false);
void videosStorageMenuHandler(bool firstStart = false);
void storageMenu();
float bytesToFloat(uint8_t* farray);
float celciusToFahrenheit(float Tc);
void frameFilename(char* filename, uint16_t count);
void liveMode();
void loadThermal();
void loadRawData(char* filename);
void settingsMenuHandler();
void connectionMenu();
void connectionMenuHandler();
void saveThermalImage(char* filename);
uint16_t getVideoFrameNumber(char* dirname);
bool checkSDCard();
void getTemperatures(bool save = false);
void liveModeHelper();
uint16_t calcAverage();
void quickCalibration();
void selectColorScheme();
void changeColorScheme(int pos);
void toggleColorbar();