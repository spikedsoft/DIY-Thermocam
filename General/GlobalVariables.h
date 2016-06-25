/*
* Global variable declarations, that are used firmware-wide
*/

#include "GlobalDefines.h"

/* Variables */

//Display Controller
UTFT display;
//Touch Controller
Touchscreen touch;
//ADC
ADC *batMeasure;
//Buttons
UTFT_Buttons touchButtons(&display, &touch);
//Button Debouncer
Bounce buttonDebouncer(pin_button, 100);
Bounce touchDebouncer(pin_touch_irq, 100);
//Fonts
extern uint8_t smallFont[];
extern uint8_t bigFont[];
//SD
SdFat sd;
SdFile sdFile;
String sdInfo;
//Camera
Camera cam(&Serial1);

//Current color scheme - standard is rainbow
byte colorScheme;
//Pointer to the current color scheme
const byte *colorMap;
//Number of rgb elements inside the color scheme
int16_t colorElements;

//160x120 image storage
unsigned short* image;
//80x60 raw values storage
unsigned short* rawValues;
//Array to store the printed temperatures
uint16_t* showTemp;

//HW diagnostic information
byte diagnostic;
//Battery
int8_t batPercentage;
//MLX90614 sensor version
bool mlx90614Version;
//FLIR Lepton sensor version
byte leptonVersion;
//Temperature format
bool tempFormat;
//Laser state
bool laserEnabled;
//Display mode
byte displayMode;
//Variables for color calculation
uint16_t maxTemp;
uint16_t minTemp;

//If enabled, image will be converted to bitmap every time
bool convertEnabled;
//If enabled, visual image will be saved automatically
bool visualEnabled;
//Automatic gain control
bool agcEnabled;
//Lock current limits
bool limitsLocked;
//Display rotation enabled
bool rotationEnabled;

//Display options
bool batteryEnabled;
bool timeEnabled;
bool dateEnabled;
bool spotEnabled;
bool colorbarEnabled;
bool pointsEnabled;
bool storageEnabled;
bool filterEnabled;
bool ambientEnabled;

//Calibration offset
float calOffset;
//Calibration slope
float calSlope;
//Calibration status
byte calStatus;
//Calibration warmup timer
long calTimer;

//Save Image in the next cycle
volatile byte imgSave;
//Save Video in the next cycle
volatile bool videoSave;
//Show Live Mode Menu in the next cycle
volatile bool showMenu;
//Release or lock limits in the next cycle
volatile bool lockLimits;