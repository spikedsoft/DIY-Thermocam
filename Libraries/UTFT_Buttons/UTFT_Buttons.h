#ifndef UTFT_Buttons_h
#define UTFT_Buttons_h

#include "Arduino.h"
#include "../UTFT/UTFT.h"
#include "../Touchscreen/Touchscreen.h"

#define MAX_BUTTONS	20	// Maximum number of buttons available at one time

// Define presets for button status
#define BUTTON_DISABLED			0x0001
#define BUTTON_SYMBOL			0x0002
#define BUTTON_SYMBOL_REP_3X	0x0004
#define BUTTON_BITMAP			0x0008	
#define BUTTON_NO_BORDER		0x0010
#define BUTTON_UNUSED			0x8000

typedef struct {
	uint16_t pos_x, pos_y, width, height;
	uint16_t flags;
	bool largetouch;
	char *label;bitmapdatatype data;
}button_type;

class UTFT_Buttons {
public:
	UTFT_Buttons(UTFT *ptrUTFT, Touchscreen *ptrTouch);

	int addButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
			char *label, uint16_t flags = 0, bool largetouch = false);
	int addButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
			bitmapdatatype data, uint16_t flags=0);
			void drawButtons();
			void drawButton(int buttonID);
			void enableButton(int buttonID, boolean redraw=false);
			void disableButton(int buttonID, boolean redraw=false);
			void relabelButton(int buttonID, char *label, boolean redraw=false);
			boolean buttonEnabled(int buttonID);
			void setActive(int buttonID);
			void setInactive(int buttonID);
			void deleteButton(int buttonID);
			void deleteAllButtons();
			int checkButtons(bool timeout = false, bool fast = false);
			void setTextFont(uint8_t* font);
			void setSymbolFont(uint8_t* font);
			void setButtonColors(word atxt, word iatxt, word brd, word brdhi, word back);

		protected:
			UTFT *_UTFT;
			Touchscreen *_Touchscreen;
			button_type buttons[MAX_BUTTONS];
			word _color_text, _color_text_inactive, _color_background, _color_border, _color_hilite;
			uint8_t *_font_text, *_font_symbol;
		};

#endif
