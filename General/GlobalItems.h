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

/* EEPROM defines */
#define eeprom_firstStart 100
#define eeprom_tempFormat 101
#define eeprom_colorScheme 102
#define eeprom_imagesFormat 103
#define eeprom_imagesType 104
#define eeprom_videosFormat 105
#define eeprom_videosType 106
#define eeprom_massStorage 107
#define eeprom_spotEnabled 108
#define eeprom_filterEnabled 109
#define eeprom_colorbarEnabled 110
#define eeprom_hwRevision 150
#define eeprom_setValue 200

/* Variables */
//Display Controller
UTFT display;
//Touch Controller
XPT2046_Touchscreen touch(9);
//ADC
ADC *batMeasure = new ADC();
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
//MLX90614 sensor version - 0 = BCI (Early-Bird #1), 1 = DCH (All other)
bool mlx90614Version;
//FLIR Lepton sensor version - 0 = Lepton2, 1 = Lepton3
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
bool filterEnabled = true;
//Laser state
bool laserEnabled = false;
//Show spot
bool spotEnabled = true;
//Show colorbar
bool colorbarEnabled = true;
//Quick calibration offset
float quickCalOffset;
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