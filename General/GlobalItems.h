/*
* Pin, Variable and Method definitions that are used firmware-wide
*/

/* Pin defines */

//General defines
#define pin_button 2
#define pin_laser 4
#define pin_touch_irq 5
#define pin_lcd_dc 6
#define pin_cam_si 8
#define pin_touch_cs 9
#define pin_flash_cs 10
#define pin_mosi 11
#define pin_miso 12
#define pin_sck 13
#define pin_alt_sck 14
#define pin_lepton_cs 15
#define pin_sda 18
#define pin_scl 19
#define pin_sd_cs 20
#define pin_lcd_cs 21
#define pin_lcd_backlight 22
#define pin_bat_measure 23
#define pin_usb_measure A14
#define pin_vref 39

/* EEPROM defines */
#define eeprom_firstStart 100
#define eeprom_tempFormat 101
#define eeprom_colorScheme 102
#define eeprom_imagesFormat 103
#define eeprom_imagesType 104
#define eeprom_videosFormat 105
#define eeprom_videosType 106
#define eeprom_massStorage 107
#define eeprom_hwRevision 150
#define eeprom_setValue 200

/* Variables */
//Display Controller
UTFT display;
//Touch Controller
XPT2046_Touchscreen touch(9);
//Buttons
UTFT_Buttons touchButtons(&display, &touch);
//Button Debouncer
Bounce buttonDebouncer = Bounce(pin_button, 100);
//Fonts
extern uint8_t smallFont[];
extern uint8_t bigFont[];
//SD
SdFat sd;
SdFile sdFile;
String sdInfo = " - / - MB";
//Camera
Camera cam(&Serial1);
//Battery
int batPercentage = -1;
long batRefreshTime;
//MLX90614 sensor version - 0 = BCI (Early-Bird #1), 1 = DCH (Early-Bird #2)
bool mlx90614Version;
//FLIR Lepton sensor version - 0 = Lepton2 (Early-Bird #1 & Early-Bird #2), 1 = Lepton3 (Batch #1)
bool leptonVersion;
//Temperature format - 0 = dec, 1 = fahrenheit
bool tempFormat = 0;
//Current color scheme - 0 = rainbow, 1 = ironblack, 2 = grayscale, 3 = hot, 4 = cold
byte colorScheme = 0;
//Images format - 0 = raw only, 1 = raw + bitmap
bool imagesFormat = 0;
//Images type - 0 = thermal only, 1 = thermal + visual
bool imagesType = 0;
//Videos format - 0 = raw only, 1 = raw + bitmap
bool videosFormat = 0;
//Videos type - 0 = thermal only, 1 = thermal + visual
bool videosType = 0;
//160x120 image storage
unsigned short* image;
//80x60 raw values storage
unsigned short* rawValues;
//Pointer to the current color scheme
const byte *colormap;
//Variables for color calculation
uint16_t maxTemp;
uint16_t minTemp;
//For hot & cold mode
byte grayscaleLevel = 85;
//Live Menu position
byte liveMenuPos = 0;
//Automatic gain control
bool agcEnabled = true;
//Calibration done
bool calibrationDone = false;
//Filter image
bool filterEnabled = false;
//Laser state
bool laserEnabled = false;
//Show spot
bool spotEnabled = true;
//Calibration formula
double calSlope;
double calOffset;
//Save Image in the next cycle
volatile byte imgSave = false;
//Show Live Mode Menu in the next cycle
volatile bool showMenu = false;
//Save Video in the next cycle
volatile bool videoSave = false;
//Video save interval in seconds
int16_t videoInterval;

/* Abstract Methods */
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
void liveModeHelper();
void selectColorScheme();