/*
* Global defines, that are used firmware-wide
*/

//Pins
#define pin_button 2
#define pin_laser 4
#define pin_touch_irq 5
#define pin_lcd_dc 6
#define pin_cam_si 8
#define pin_touch_cs 9 //Only resistive touch
#define pin_flash_cs 10 //Not in use
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

//Modes
#define displayMode_thermal 0
#define displayMode_visual 1
#define displayMode_combined 2

//MLX90614 sensor version
#define mlx90614Version_old 0 //MLX90614-BCI
#define mlx90614Version_new 1 //MLX90614-DCI, MLX90614-BCF, MLX90614-DCH

//FLIR Lepton sensor version
#define leptonVersion_2_Shutter 0 //FLIR Lepton2 Shuttered
#define leptonVersion_3_Shutter 1 //FLIR Lepton3 Shuttered
#define leptonVersion_2_NoShutter 2 //FLIR Lepton2 Non-Shuttered

//Temperature format
#define tempFormat_celcius 0
#define tempFormat_fahrenheit 1

//Filter type
#define filterType_none 0
#define filterType_gaussian 1
#define filterType_box 2

//Shutter mode
#define shutterMode_manual 0
#define shutterMode_auto 1
#define shutterMode_none 2

//EEPROM registers
#define eeprom_tempFormat 101
#define eeprom_colorScheme 102
#define eeprom_convertEnabled 103
#define eeprom_visualEnabled 104
#define eeprom_massStorage 107
#define eeprom_spotEnabled 108
#define eeprom_filterType 109
#define eeprom_colorbarEnabled 110
#define eeprom_batteryEnabled 111
#define eeprom_timeEnabled 112
#define eeprom_dateEnabled 113
#define eeprom_pointsEnabled 114
#define eeprom_storageEnabled 115
#define eeprom_rotationEnabled 116
#define eeprom_displayMode 117
#define eeprom_calSlopeSet 130
#define eeprom_calSlopeBase 131 //4 Byte (131-134)
#define eeprom_calOffsetSet 135
#define eeprom_calOffsetBase 136 //4 Byte (136-139)
#define eeprom_firstStart 150
#define eeprom_liveHelper 151
#define eeprom_fwVersion 250
#define eeprom_setValue 200

//Hardware diagnostic bit codes
#define diag_spot 0
#define diag_display 1
#define diag_camera 2
#define diag_touch 3
#define diag_sd 4
#define diag_bat 5
#define diag_lep_conf 6
#define diag_lep_data 7
#define diag_ok 255

//Color scheme numbers
#define colorSchemeTotal 19
#define colorScheme_arctic 0
#define colorScheme_blackHot 1
#define colorScheme_blueRed 2
#define colorScheme_coldest 3
#define colorScheme_contrast 4
#define colorScheme_doubleRainbow 5
#define colorScheme_grayRed 6
#define colorScheme_glowBow 7
#define colorScheme_grayscale 8
#define colorScheme_hottest 9
#define colorScheme_ironblack 10
#define colorScheme_lava 11
#define colorScheme_medical 12
#define colorScheme_rainbow 13
#define colorScheme_wheel1 14
#define colorScheme_wheel2 15
#define colorScheme_wheel3 16
#define colorScheme_whiteHot 17
#define colorScheme_yellow 18

//Calibration
#define cal_warmup 0
#define cal_standard 1
#define cal_manual 2
#define cal_stdSlope 0.0402f //Standard slope value

//Image save marker
#define imgSave_disabled 0
#define imgSave_save 1
#define imgSave_set 2
#define imgSave_create 3


