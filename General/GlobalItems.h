/*
* Global definitions that are used firmware-wide
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

/* EEPROM defines */
#define eeprom_tempFormat 101
#define eeprom_colorScheme 102
#define eeprom_convertEnabled 103
#define eeprom_visualEnabled 104
#define eeprom_massStorage 107
#define eeprom_spotEnabled 108
#define eeprom_filterEnabled 109
#define eeprom_colorbarEnabled 110
#define eeprom_batteryEnabled 111
#define eeprom_timeEnabled 112
#define eeprom_dateEnabled 113
#define eeprom_pointsEnabled 114
#define eeprom_storageEnabled 115
#define eeprom_firstStart 150
#define eeprom_setValue 200

/* Variables */

//160x120 image storage
unsigned short* image;
//80x60 raw values storage
unsigned short* rawValues;
//Array to store the printed temperatures
uint16_t* showTemp;

//Display Controller
UTFT display;
//Touch Controller
Touchscreen touch;
//ADC
ADC *batMeasure = new ADC();
//Buttons
UTFT_Buttons touchButtons(&display, &touch);
//Button Debouncer
Bounce buttonDebouncer = Bounce(pin_button, 100);
Bounce touchDebouncer = Bounce(pin_touch_irq, 100);
//Fonts
extern uint8_t smallFont[];
extern uint8_t bigFont[];
//SD
SdFat sd;
SdFile sdFile;
String sdInfo;
//Camera
Camera cam(&Serial1);
//Battery
int8_t batPercentage = -1;

//MLX90614 sensor version - 0 = BCI (Early-Bird #1), 1 = DCH (All other)
bool mlx90614Version;
//FLIR Lepton sensor version - 0 = Lepton2 Shuttered, 1 = Lepton3 Shuttered, 2 = Lepton2 No-Shutter 
byte leptonVersion;
//Temperature format - 0 = dec, 1 = fahrenheit
bool tempFormat = 0;
//Current color scheme - 0 = rainbow, 1 = ironblack, 2 = grayscale, 3 = hot, 4 = cold
byte colorScheme = 0;
//Pointer to the current color scheme
const byte *colorMap;
//Number of rgb elements inside the color scheme
int16_t colorElements;
//Variables for color calculation
uint16_t maxTemp;
uint16_t minTemp;
//For hot & cold mode
byte grayscaleLevel = 85;
//If enabled, image will be converted to bitmap every time
bool convertEnabled = false;
//If enabled, visual image will be saved automatically
bool visualEnabled = false;
//Automatic gain control
bool agcEnabled = true;
//Lock current limits
bool limitsLocked = false;

//Display options
bool batteryEnabled = false;
bool timeEnabled = false;
bool dateEnabled = false;
bool spotEnabled = true;
bool colorbarEnabled = true;
bool pointsEnabled = true;
bool storageEnabled = false;
bool filterEnabled = true;
//Laser state
bool laserEnabled = false;

//Calibration coefficients
float calOffset;
float calSlope = 0.025;
//Calibration status - 0 = warmup, 1 = standard coeff, 2 = manual coeff
byte calStatus = 0;
//Calibration warmup timer
long calTimer;

//Save Image in the next cycle
volatile byte imgSave = false;
//Save Video in the next cycle
volatile bool videoSave = false;
//Show Live Mode Menu in the next cycle
volatile bool showMenu = false;
//Release or lock limits in the next cycle
volatile bool lockLimits = false;