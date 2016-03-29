#ifndef UTFT_h
#define UTFT_h

#define SPICLOCK 40000000

#define UTFT_VERSION 270
#define VERSION9341 12

#define LEFT 0
#define RIGHT 9999
#define CENTER 9998

#define PORTRAIT 0
#define LANDSCAPE 1

//pinout defs
#define LED 22
#define CS 21
#define DC 6

#define ILI9341_TFTWIDTH  240
#define ILI9341_TFTHEIGHT 320

#define ILI9341_NOP     0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09

#define ILI9341_SLPIN   0x10
#define ILI9341_SLPOUT  0x11
#define ILI9341_PTLON   0x12
#define ILI9341_NORON   0x13

#define ILI9341_RDMODE  0x0A
#define ILI9341_RDMADCTL  0x0B
#define ILI9341_RDPIXFMT  0x0C
#define ILI9341_RDIMGFMT  0x0A
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_INVOFF  0x20
#define ILI9341_INVON   0x21
#define ILI9341_GAMMASET 0x26
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON  0x29

#define ILI9341_CASET   0x2A
#define ILI9341_PASET   0x2B
#define ILI9341_RAMWR   0x2C
#define ILI9341_RAMRD   0x2E

#define ILI9341_PTLAR   0x30
#define ILI9341_MADCTL  0x36
#define ILI9341_PIXFMT  0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR  0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1  0xC0
#define ILI9341_PWCTR2  0xC1
#define ILI9341_PWCTR3  0xC2
#define ILI9341_PWCTR4  0xC3
#define ILI9341_PWCTR5  0xC4
#define ILI9341_VMCTR1  0xC5
#define ILI9341_VMCTR2  0xC7

#define ILI9341_RDID1   0xDA
#define ILI9341_RDID2   0xDB
#define ILI9341_RDID3   0xDC
#define ILI9341_RDID4   0xDD

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1

//*********************************
// COLORS
//*********************************
// VGA color palette
#define VGA_BLACK		0x0000
#define VGA_WHITE		0xFFFF
#define VGA_RED			0xF800
#define VGA_GREEN		0x0400
#define VGA_BLUE		0x001F
#define VGA_SILVER		0xC618
#define VGA_GRAY		0x8410
#define VGA_MAROON		0x8000
#define VGA_YELLOW		0xFFE0
#define VGA_OLIVE		0x8400
#define VGA_LIME		0x07E0
#define VGA_AQUA		0x07FF
#define VGA_TEAL		0x0410
#define VGA_NAVY		0x0010
#define VGA_FUCHSIA		0xF81F
#define VGA_PURPLE		0x8010
#define VGA_TRANSPARENT	0xFFFFFFFF

#include "Arduino.h"
#include "../SPI/SPI.h"

#define swap(type, i, j) {type t = i; i = j; j = t;}
#define fontbyte(x) cfont.font[x]
#define bitmapdatatype unsigned short*

struct _current_font {
	uint8_t* font;
	uint8_t x_size;
	uint8_t y_size;
	uint8_t offset;
	uint8_t numchars;
};

static const uint8_t init_commands[] = { 4, 0xEF, 0x03, 0x80, 0x02, 4, 0xCF,
0x00, 0XC1, 0X30, 5, 0xED, 0x64, 0x03, 0X12, 0X81, 4, 0xE8, 0x85, 0x00,
0x78, 6, 0xCB, 0x39, 0x2C, 0x00, 0x34, 0x02, 2, 0xF7, 0x20, 3, 0xEA,
0x00, 0x00, 2, ILI9341_PWCTR1,
0x23, // Power control
2, ILI9341_PWCTR2,
0x10, // Power control
3, ILI9341_VMCTR1, 0x3e,
0x28, // VCM control
2, ILI9341_VMCTR2,
0x86, // VCM control2
2, ILI9341_MADCTL,
0x48, // Memory Access Control
2, ILI9341_PIXFMT, 0x55, 3, ILI9341_FRMCTR1, 0x00, 0x18, 4,
ILI9341_DFUNCTR, 0x08, 0x82,
0x27, // Display Function Control
2, 0xF2,
0x00, // Gamma Function Disable
2, ILI9341_GAMMASET,
0x01, // Gamma curve selected
16, ILI9341_GMCTRP1, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
0x37, 0x07, 0x10, 0x03, 0x0E, 0x09,
0x00, // Set Gamma
16, ILI9341_GMCTRN1, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F, // Set Gamma
0 };

class UTFT {
public:
	void InitLCD(byte orientation = LANDSCAPE);
	void clrScr();
	void drawPixel(int x, int y);
	void drawLine(int x1, int y1, int x2, int y2);
	void fillScr(byte r, byte g, byte b);
	void fillScr(word color);
	void drawRect(int x1, int y1, int x2, int y2);
	void fillRect(int x1, int y1, int x2, int y2);

	void drawRoundRect(int x1, int y1, int x2, int y2);
	void fillRoundRect(int x1, int y1, int x2, int y2);
	void drawCircle(int x, int y, int radius);
	void fillCircle(int x, int y, int radius);

	void setColor(byte r, byte g, byte b);
	void setColor(word color);
	word getColor();
	void setBackColor(byte r, byte g, byte b);
	void setBackColor(uint32_t color);
	word getBackColor();
	void print(char *st, int x, int y, int deg = 0);
	void printC(String st, int x, int y, uint32_t color = VGA_WHITE);
	void print(String st, int x, int y, int deg = 0);
	void rotateChar(byte c, int x, int y, int pos, int deg);
	void printNumI(long num, int x, int y, int length = 0, char filler = ' ');
	void printNumF(double num, byte dec, int x, int y, char divider = '.',
		int length = 0, char filler = ' ');
	void setFont(uint8_t* font);
	uint8_t* getFont();
	uint8_t getFontXsize();
	uint8_t getFontYsize();
	void drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int scale = 1);
	void drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int deg, int rox, int roy);
	void lcdOff();
	void lcdOn();
	void setRotation(uint8_t m);
	//void setContrast(char c);
	int  getDisplayXSize();
	int	 getDisplayYSize();

	/*
	The functions and variables below should not normally be used.
	They have been left publicly available for use in add-on libraries
	that might need access to the lower level functions of UTFT.

	Please note that these functions and variables are not documented
	and I do not provide support on how to use them.
	*/
	char		imgbuf[160];
	byte __p1, __p2, __p3, __p4;
	byte fch, fcl, bch, bcl;
	byte orient;
	long disp_x_size, disp_y_size;
	//byte display_model, display_transfer_mode, display_serial_mode;
	//regtype *P_RS, *P_WR, *P_CS, *P_RST, *P_SDA, *P_SCL, *P_ALE;
	//regsize B_RS, B_WR, B_CS, B_RST, B_SDA, B_SCL, B_ALE;
	_current_font	cfont;
	boolean _transparent;
	uint8_t rotation;

	//void LCD_Writ_Bus(char VH,char VL, byte mode);
	void LCD_Write_DATA(char VH, char VL);
	void setPixel(word color);
	void drawHLine(int x, int y, int l);
	void drawVLine(int x, int y, int l);
	void printChar(byte c, int x, int y);
	void setXY(word x1, word y1, word x2, word y2);
	void clrXY();
	void _convert_float(char *buf, double num, int width, byte prec);

	void enterSleepMode() {
		SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
		writecommand_last(ILI9341_SLPIN);
		SPI.endTransaction();
	}

	void exitSleepMode() {
		SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
		writecommand_last(ILI9341_SLPOUT);
		SPI.endTransaction();
	}

	//Write RGB565 data to the screen
	void writeScreen(unsigned short *pcolors)
	{
		SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
		setAddr(0, 0, 319, 239);
		writecommand_cont(ILI9341_RAMWR);
		for (byte y = 120; y > 0; y--) {
			for (byte x = 160; x > 1; x--) {
				writedata16_cont(*pcolors);
				writedata16_cont(*pcolors++);
			}
			writedata16_cont(*pcolors);
			writedata16_last(*pcolors++);
			pcolors = pcolors - 160;
			for (byte x = 160; x > 1; x--) {
				writedata16_cont(*pcolors);
				writedata16_cont(*pcolors++);
			}
			writedata16_cont(*pcolors);
			writedata16_last(*pcolors++);
		}
		SPI.endTransaction();
	}

	// Now lets see if we can read in multiple pixels
	void readScreen(byte step, unsigned short *pcolors)
	{
		uint8_t dummy __attribute__((unused));
		uint8_t r, g, b;
		uint16_t c = 19201;

		SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

		setAddr(0, (step * 60), 319, (step * 60) + 59);
		writecommand_cont(ILI9341_RAMRD); // read from RAM
		waitTransmitComplete();
		KINETISK_SPI0.PUSHR = 0 | (pcs_data << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT | SPI_PUSHR_EOQ;
		while ((KINETISK_SPI0.SR & SPI_SR_EOQF) == 0);
		KINETISK_SPI0.SR = SPI_SR_EOQF;  // make sure it is clear
		while ((KINETISK_SPI0.SR & 0xf0)) {
			dummy = KINETISK_SPI0.POPR;	// Read a DUMMY byte but only once
		}
		c *= 3; // number of bytes we will transmit to the display
		while (c--) {
			if (c) {
				KINETISK_SPI0.PUSHR = 0 | (pcs_data << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;
			}
			else {
				KINETISK_SPI0.PUSHR = 0 | (pcs_data << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_EOQ;
			}

			// If last byte wait until all have come in...
			if (c == 0) {
				while ((KINETISK_SPI0.SR & SPI_SR_EOQF) == 0);
				KINETISK_SPI0.SR = SPI_SR_EOQF;  // make sure it is clear
			}

			if ((KINETISK_SPI0.SR & 0xf0) >= 0x30) { // do we have at least 3 bytes in queue if so extract...
				r = KINETISK_SPI0.POPR;		// Read a RED byte of GRAM
				g = KINETISK_SPI0.POPR;		// Read a GREEN byte of GRAM
				b = KINETISK_SPI0.POPR;		// Read a BLUE byte of GRAM
				*pcolors++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
			}

			// like waitFiroNotFull but does not pop our return queue
			while ((KINETISK_SPI0.SR & (15 << 12)) > (3 << 12));
		}
		SPI.endTransaction();
	}


private:

	uint8_t pcs_data, pcs_command;

	void setAddr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
		__attribute__((always_inline)) {
		writecommand_cont(ILI9341_CASET); // Column addr set
		writedata16_cont(x0);   // XSTART
		writedata16_cont(x1);   // XEND
		writecommand_cont(ILI9341_PASET); // Row addr set
		writedata16_cont(y0);   // YSTART
		writedata16_cont(y1);   // YEND
	}
	//void waitFifoNotFull(void) __attribute__((always_inline)) {
	void waitFifoNotFull(void) {
		uint32_t sr;
		uint32_t tmp __attribute__((unused));
		do {
			sr = KINETISK_SPI0.SR;
			if (sr & 0xF0) tmp = KINETISK_SPI0.POPR;  // drain RX FIFO
		} while ((sr & (15 << 12)) > (3 << 12));
	}
	void waitFifoEmpty(void) {
		uint32_t sr;
		uint32_t tmp __attribute__((unused));
		do {
			sr = KINETISK_SPI0.SR;
			if (sr & 0xF0) tmp = KINETISK_SPI0.POPR;  // drain RX FIFO
		} while ((sr & 0xF0F0) > 0);             // wait both RX & TX empty
	}
	void waitTransmitComplete(void) __attribute__((always_inline)) {
		uint32_t tmp __attribute__((unused));
		while (!(KINETISK_SPI0.SR & SPI_SR_TCF)); // wait until final output done
		tmp = KINETISK_SPI0.POPR;                  // drain the final RX FIFO word
	}
	void waitTransmitComplete(uint32_t mcr) __attribute__((always_inline)) {
		uint32_t tmp __attribute__((unused));
		while (1) {
			uint32_t sr = KINETISK_SPI0.SR;
			if (sr & SPI_SR_EOQF) break;  // wait for last transmit
			if (sr & 0xF0) tmp = KINETISK_SPI0.POPR;
		}
		KINETISK_SPI0.SR = SPI_SR_EOQF;
		SPI0_MCR = mcr;
		while (KINETISK_SPI0.SR & 0xF0) {
			tmp = KINETISK_SPI0.POPR;
		}
	}
	void writecommand_cont(uint8_t c) __attribute__((always_inline)) {
		KINETISK_SPI0.PUSHR = c | (pcs_command << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;
		waitFifoNotFull();
	}
	void writedata8_cont(uint8_t c) __attribute__((always_inline)) {
		KINETISK_SPI0.PUSHR = c | (pcs_data << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_CONT;
		waitFifoNotFull();
	}
	void writedata16_cont(uint16_t d) __attribute__((always_inline)) {
		KINETISK_SPI0.PUSHR = d | (pcs_data << 16) | SPI_PUSHR_CTAS(1) | SPI_PUSHR_CONT;
		waitFifoNotFull();
	}
	void writecommand_last(uint8_t c) __attribute__((always_inline)) {
		uint32_t mcr = SPI0_MCR;
		KINETISK_SPI0.PUSHR = c | (pcs_command << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_EOQ;
		waitTransmitComplete(mcr);
	}
	void writedata8_last(uint8_t c) __attribute__((always_inline)) {
		uint32_t mcr = SPI0_MCR;
		KINETISK_SPI0.PUSHR = c | (pcs_data << 16) | SPI_PUSHR_CTAS(0) | SPI_PUSHR_EOQ;
		waitTransmitComplete(mcr);
	}
	void writedata16_last(uint16_t d) __attribute__((always_inline)) {
		uint32_t mcr = SPI0_MCR;
		KINETISK_SPI0.PUSHR = d | (pcs_data << 16) | SPI_PUSHR_CTAS(1) | SPI_PUSHR_EOQ;
		waitTransmitComplete(mcr);
	}
	void HLine(int16_t x, int16_t y, int16_t w, uint16_t color)
		__attribute__((always_inline)) {
		setAddr(x, y, x + w - 1, y);
		writecommand_cont(ILI9341_RAMWR);
		do {
			writedata16_cont(color);
		} while (--w > 0);
	}
	void VLine(int16_t x, int16_t y, int16_t h, uint16_t color)
		__attribute__((always_inline)) {
		setAddr(x, y, x, y + h - 1);
		writecommand_cont(ILI9341_RAMWR);
		do {
			writedata16_cont(color);
		} while (--h > 0);
	}
	void Pixel(int16_t x, int16_t y, uint16_t color)
		__attribute__((always_inline)) {
		setAddr(x, y, x, y);
		writecommand_cont(ILI9341_RAMWR);
		writedata16_cont(color);
	}

};

#endif
