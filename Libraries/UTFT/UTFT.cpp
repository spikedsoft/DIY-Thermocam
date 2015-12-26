#include "UTFT.h"

#define TFT_BL_OFF digitalWriteFast(LED,LOW) 
#define TFT_BL_ON  digitalWriteFast(LED,HIGH)


void UTFT::LCD_Write_DATA(char VH, char VL) {
	setPixel((VH << 8) | VL);
}

void UTFT::_convert_float(char *buf, double num, int width, byte prec)
{
	dtostrf(num, width, prec, buf);
}


void UTFT::InitLCD(byte orientation) {

	pinMode(LED, OUTPUT);

	disp_x_size = 319;
	disp_y_size = 239;

	TFT_BL_ON;
	orient = orientation;

	if (SPI.pinIsChipSelect(CS, DC)) {
		pcs_data = SPI.setCS(CS);
		pcs_command = pcs_data | SPI.setCS(DC);
	} else {
		pcs_data = 0;
		pcs_command = 0;
		return;
	}

	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	const uint8_t *addr = init_commands;
	while (1) {
		uint8_t count = *addr++;
		if (count-- == 0)
			break;
		writecommand_cont(*addr++);
		while (count-- > 0) {
			writedata8_cont(*addr++);
		}
	}
	writecommand_last(ILI9341_SLPOUT);    // Exit Sleep
	SPI.endTransaction();

	delay(120);
	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	writecommand_last(ILI9341_DISPON);    // Display on
	SPI.endTransaction();

	cfont.font = 0;
	_transparent = false;
	setRotation(45);
}

void UTFT::setXY(word x1, word y1, word x2, word y2) {
	if (orient == LANDSCAPE) {
		swap(word, x1, y1);
		swap(word, x2, y2)
		y1 = disp_y_size - y1;
		y2 = disp_y_size - y2;
		swap(word, y1, y2)
	}

	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	setAddr(x1, y1, x2, y2);
	writecommand_last(ILI9341_RAMWR); // write to RAM
	SPI.endTransaction();
}

void UTFT::clrXY() {
	if (orient == PORTRAIT)
		setXY(0, 0, disp_x_size, disp_y_size);
	else
		setXY(0, 0, disp_y_size, disp_x_size);
}

void UTFT::drawRect(int x1, int y1, int x2, int y2) {
	if (x1 > x2) {
		swap(int, x1, x2);
	}
	if (y1 > y2) {
		swap(int, y1, y2);
	}

	drawHLine(x1, y1, x2 - x1);
	drawHLine(x1, y2, x2 - x1);
	drawVLine(x1, y1, y2 - y1);
	drawVLine(x2, y1, y2 - y1);
}

void UTFT::fillRect(int x1, int y1, int x2, int y2) {
	if (x1 > x2) {
		swap(int, x1, x2);
	}
	if (y1 > y2) {
		swap(int, y1, y2);
	}

	int w = x2 - x1;
	int h = y2 - y1;
	// rudimentary clipping (drawChar w/big text requires this)
	if ((x1 >= disp_x_size + 1) || (y1 >= disp_y_size + 1))
		return;
	if ((x1 + w - 1) >= disp_x_size + 1)
		w = disp_x_size + 1 - x1;
	if ((y1 + h - 1) >= disp_y_size + 1)
		h = disp_y_size + 1 - y1;

	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	setAddr(x1, y1, x1 + w - 1, y1 + h - 1);
	writecommand_cont(ILI9341_RAMWR);
	word color = (fch << 8 | fcl);
	for (int y = h; y > 0; y--) {
		for (int x = w; x > 1; x--) {
			writedata16_cont(color);
		}
		writedata16_last(color);
	}
	SPI.endTransaction();
}

void UTFT::drawRoundRect(int x1, int y1, int x2, int y2) {
	if (x1 > x2) {
		swap(int, x1, x2);
	}
	if (y1 > y2) {
		swap(int, y1, y2);
	}
	if ((x2 - x1) > 4 && (y2 - y1) > 4) {
		drawPixel(x1 + 1, y1 + 1);
		drawPixel(x2 - 1, y1 + 1);
		drawPixel(x1 + 1, y2 - 1);
		drawPixel(x2 - 1, y2 - 1);
		drawHLine(x1 + 2, y1, x2 - x1 - 4);
		drawHLine(x1 + 2, y2, x2 - x1 - 4);
		drawVLine(x1, y1 + 2, y2 - y1 - 4);
		drawVLine(x2, y1 + 2, y2 - y1 - 4);
	}
}

void UTFT::fillRoundRect(int x1, int y1, int x2, int y2) {
	if (x1 > x2) {
		swap(int, x1, x2);
	}
	if (y1 > y2) {
		swap(int, y1, y2);
	}

	if ((x2 - x1) > 4 && (y2 - y1) > 4) {
		for (int i = 0; i < ((y2 - y1) / 2) + 1; i++) {
			switch (i) {
			case 0:
				drawHLine(x1 + 2, y1 + i, x2 - x1 - 4);
				drawHLine(x1 + 2, y2 - i, x2 - x1 - 4);
				break;
			case 1:
				drawHLine(x1 + 1, y1 + i, x2 - x1 - 2);
				drawHLine(x1 + 1, y2 - i, x2 - x1 - 2);
				break;
			default:
				drawHLine(x1, y1 + i, x2 - x1);
				drawHLine(x1, y2 - i, x2 - x1);
			}
		}
	}
}

void UTFT::drawCircle(int x, int y, int radius) {
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;

	//cbi(P_CS, B_CS);
	setXY(x, y + radius, x, y + radius);
	LCD_Write_DATA(fch, fcl);
	setXY(x, y - radius, x, y - radius);
	LCD_Write_DATA(fch, fcl);
	setXY(x + radius, y, x + radius, y);
	LCD_Write_DATA(fch, fcl);
	setXY(x - radius, y, x - radius, y);
	LCD_Write_DATA(fch, fcl);

	while (x1 < y1) {
		if (f >= 0) {
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;
		setXY(x + x1, y + y1, x + x1, y + y1);
		LCD_Write_DATA(fch, fcl);
		setXY(x - x1, y + y1, x - x1, y + y1);
		LCD_Write_DATA(fch, fcl);
		setXY(x + x1, y - y1, x + x1, y - y1);
		LCD_Write_DATA(fch, fcl);
		setXY(x - x1, y - y1, x - x1, y - y1);
		LCD_Write_DATA(fch, fcl);
		setXY(x + y1, y + x1, x + y1, y + x1);
		LCD_Write_DATA(fch, fcl);
		setXY(x - y1, y + x1, x - y1, y + x1);
		LCD_Write_DATA(fch, fcl);
		setXY(x + y1, y - x1, x + y1, y - x1);
		LCD_Write_DATA(fch, fcl);
		setXY(x - y1, y - x1, x - y1, y - x1);
		LCD_Write_DATA(fch, fcl);
	}
	//sbi(P_CS, B_CS);
	clrXY();
}

void UTFT::fillCircle(int x, int y, int radius) {
	for (int y1 = -radius; y1 <= 0; y1++)
		for (int x1 = -radius; x1 <= 0; x1++)
			if (x1 * x1 + y1 * y1 <= radius * radius) {
				drawHLine(x + x1, y + y1, 2 * (-x1));
				drawHLine(x + x1, y - y1, 2 * (-x1));
				break;
			}
}

void UTFT::clrScr() {
	int x = 0;
	int y = 0;
	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	setAddr(x, y, x + disp_x_size, y + disp_y_size);
	writecommand_cont(ILI9341_RAMWR);
	for (y = disp_y_size + 1; y > 0; y--) {
		for (x = disp_x_size + 1; x > 1; x--) {
			writedata16_cont(0x00);
		}
		writedata16_last(0x00);
	}
	SPI.endTransaction();
}

void UTFT::fillScr(byte r, byte g, byte b) {
	word color = ((r & 248) << 8 | (g & 252) << 3 | (b & 248) >> 3);
	fillScr(color);
}

void UTFT::fillScr(word color) {
	int x = 0;
	int y = 0;
	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	setAddr(x, y, x + disp_x_size, y + disp_y_size);
	writecommand_cont(ILI9341_RAMWR);
	for (y = disp_y_size + 1; y > 0; y--) {
		for (x = disp_x_size + 1; x > 1; x--) {
			writedata16_cont(color);
		}
		writedata16_last(color);
	}
	SPI.endTransaction();
}

void UTFT::setColor(byte r, byte g, byte b) {
	fch = ((r & 248) | g >> 5);
	fcl = ((g & 28) << 3 | b >> 3);
}

void UTFT::setColor(word color) {
	fch = byte(color >> 8);
	fcl = byte(color & 0xFF);
}

word UTFT::getColor() {
	return (fch << 8) | fcl;
}

void UTFT::setBackColor(byte r, byte g, byte b) {
	bch = ((r & 248) | g >> 5);
	bcl = ((g & 28) << 3 | b >> 3);
	_transparent = false;
}

void UTFT::setBackColor(uint32_t color) {
	if (color == VGA_TRANSPARENT)
		_transparent = true;
	else {
		bch = byte(color >> 8);
		bcl = byte(color & 0xFF);
		_transparent = false;
	}
}

word UTFT::getBackColor() {
	return (bch << 8) | bcl;
}

void UTFT::setPixel(word color) {
	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	writedata16_last(color);
	SPI.endTransaction();
}

void UTFT::drawPixel(int x, int y) {
	if ((x < 0) || (x >= disp_x_size + 1) || (y < 0) || (y >= disp_y_size + 1))
		return;

	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	setAddr(x, y, x, y);
	writecommand_cont(ILI9341_RAMWR);
	writedata16_last(fch << 8 | fcl);
	SPI.endTransaction();
}

void UTFT::drawLine(int x1, int y1, int x2, int y2) {
	if (y1 == y2)
		drawHLine(x1, y1, x2 - x1);
	else if (x1 == x2)
		drawVLine(x1, y1, y2 - y1);
	else {
		unsigned int dx = (x2 > x1 ? x2 - x1 : x1 - x2);
		short xstep = x2 > x1 ? 1 : -1;
		unsigned int dy = (y2 > y1 ? y2 - y1 : y1 - y2);
		short ystep = y2 > y1 ? 1 : -1;
		int col = x1, row = y1;

		if (dx < dy) {
			int t = -(dy >> 1);
			while (true) {
				setXY(col, row, col, row);
				LCD_Write_DATA(fch, fcl);
				if (row == y2)
					return;
				row += ystep;
				t += dx;
				if (t >= 0) {
					col += xstep;
					t -= dy;
				}
			}
		} else {
			int t = -(dx >> 1);
			while (true) {
				setXY(col, row, col, row);
				LCD_Write_DATA(fch, fcl);
				if (col == x2)
					return;
				col += xstep;
				t += dy;
				if (t >= 0) {
					row += ystep;
					t -= dx;
				}
			}
		}
	}
	clrXY();
}

void UTFT::drawHLine(int x, int y, int l) {
	// Rudimentary clipping
	if ((x >= disp_x_size + 1) || (y >= disp_y_size + 1))
		return;
	if ((x + l - 1) >= disp_x_size + 1)
		l = disp_x_size + 1 - x;

	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	setAddr(x, y, x + l - 1, y);
	writecommand_cont(ILI9341_RAMWR);
	word color = (fch << 8 | fcl);
	while (l-- > 1) {
		writedata16_cont(color);
	}
	writedata16_last(color);
	SPI.endTransaction();
}

void UTFT::drawVLine(int x, int y, int l) {
	// Rudimentary clipping
	if ((x >= disp_x_size + 1) || (y >= disp_y_size + 1))
		return;
	if ((y + l - 1) >= disp_y_size + 1)
		l = disp_y_size + 1 - y;

	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	setAddr(x, y, x, y + l - 1);
	writecommand_cont(ILI9341_RAMWR);
	word color = (fch << 8 | fcl);
	while (l-- > 1) {
		writedata16_cont(color);
	}
	writedata16_last(color);
	SPI.endTransaction();
}

void UTFT::printChar(byte c, int x, int y) {
	byte i, ch;
	word j;
	word temp;

	if (!_transparent) {
		if (orient == PORTRAIT) {
			setXY(x, y, x + cfont.x_size - 1, y + cfont.y_size - 1);

			temp = ((c - cfont.offset) * ((cfont.x_size / 8) * cfont.y_size))
					+ 4;
			for (j = 0; j < ((cfont.x_size / 8) * cfont.y_size); j++) {
				ch = pgm_read_byte(&cfont.font[temp]);
				for (i = 0; i < 8; i++) {
					if ((ch & (1 << (7 - i))) != 0) {
						setPixel((fch << 8) | fcl);
					} else {
						setPixel((bch << 8) | bcl);
					}
				}
				temp++;
			}
		} else {
			temp = ((c - cfont.offset) * ((cfont.x_size / 8) * cfont.y_size))
					+ 4;

			for (j = 0; j < ((cfont.x_size / 8) * cfont.y_size);
					j += (cfont.x_size / 8)) {
				setXY(x, y + (j / (cfont.x_size / 8)), x + cfont.x_size - 1,
						y + (j / (cfont.x_size / 8)));
				for (int zz = (cfont.x_size / 8) - 1; zz >= 0; zz--) {
					ch = pgm_read_byte(&cfont.font[temp + zz]);
					for (i = 0; i < 8; i++) {
						if ((ch & (1 << i)) != 0) {
							setPixel((fch << 8) | fcl);
						} else {
							setPixel((bch << 8) | bcl);
						}
					}
				}
				temp += (cfont.x_size / 8);
			}
		}
	} else {
		temp = ((c - cfont.offset) * ((cfont.x_size / 8) * cfont.y_size)) + 4;
		for (j = 0; j < cfont.y_size; j++) {
			for (int zz = 0; zz < (cfont.x_size / 8); zz++) {
				ch = pgm_read_byte(&cfont.font[temp + zz]);
				for (i = 0; i < 8; i++) {
					setXY(x + i + (zz * 8), y + j, x + i + (zz * 8) + 1,
							y + j + 1);

					if ((ch & (1 << (7 - i))) != 0) {
						setPixel((fch << 8) | fcl);
					}
				}
			}
			temp += (cfont.x_size / 8);
		}
	}

	clrXY();
}

void UTFT::rotateChar(byte c, int x, int y, int pos, int deg) {
	byte i, j, ch;
	word temp;
	int newx, newy;
	double radian;
	radian = deg * 0.0175;

	temp = ((c - cfont.offset) * ((cfont.x_size / 8) * cfont.y_size)) + 4;
	for (j = 0; j < cfont.y_size; j++) {
		for (int zz = 0; zz < (cfont.x_size / 8); zz++) {
			ch = pgm_read_byte(&cfont.font[temp + zz]);
			for (i = 0; i < 8; i++) {
				newx = x
						+ (((i + (zz * 8) + (pos * cfont.x_size)) * cos(radian))
								- ((j) * sin(radian)));
				newy = y
						+ (((j) * cos(radian))
								+ ((i + (zz * 8) + (pos * cfont.x_size))
										* sin(radian)));

				setXY(newx, newy, newx + 1, newy + 1);

				if ((ch & (1 << (7 - i))) != 0) {
					setPixel((fch << 8) | fcl);
				} else {
					if (!_transparent)
						setPixel((bch << 8) | bcl);
				}
			}
		}
		temp += (cfont.x_size / 8);
	}
	clrXY();
}

void UTFT::print(char *st, int x, int y, int deg) {
	int stl, i;

	stl = strlen(st);

	if (orient == PORTRAIT) {
		if (x == RIGHT)
			x = (disp_x_size + 1) - (stl * cfont.x_size);
		if (x == CENTER)
			x = ((disp_x_size + 1) - (stl * cfont.x_size)) / 2;
	} else {
		if (x == RIGHT)
			x = (disp_y_size + 1) - (stl * cfont.x_size);
		if (x == CENTER)
			x = ((disp_y_size + 1) - (stl * cfont.x_size)) / 2;
	}

	for (i = 0; i < stl; i++)
		if (deg == 0)
			printChar(*st++, x + (i * (cfont.x_size)), y);
		else
			rotateChar(*st++, x, y, i, deg);
}

void UTFT::printC(String st, int x, int y, uint32_t color) {
	char buf[st.length() + 1];
	setColor(color);
	st.toCharArray(buf, st.length() + 1);
	print(buf, x, y, 0);
}

void UTFT::print(String st, int x, int y, int deg) {
	char buf[st.length() + 1];
	st.toCharArray(buf, st.length() + 1);
	print(buf, x, y, deg);
}

void UTFT::printNumI(long num, int x, int y, int length, char filler) {
	char buf[25];
	char st[27];
	boolean neg = false;
	int c = 0, f = 0;

	if (num == 0) {
		if (length != 0) {
			for (c = 0; c < (length - 1); c++)
				st[c] = filler;
			st[c] = 48;
			st[c + 1] = 0;
		} else {
			st[0] = 48;
			st[1] = 0;
		}
	} else {
		if (num < 0) {
			neg = true;
			num = -num;
		}

		while (num > 0) {
			buf[c] = 48 + (num % 10);
			c++;
			num = (num - (num % 10)) / 10;
		}
		buf[c] = 0;

		if (neg) {
			st[0] = 45;
		}

		if (length > (c + neg)) {
			for (int i = 0; i < (length - c - neg); i++) {
				st[i + neg] = filler;
				f++;
			}
		}

		for (int i = 0; i < c; i++) {
			st[i + neg + f] = buf[c - i - 1];
		}
		st[c + neg + f] = 0;

	}

	print(st, x, y);
}

void UTFT::printNumF(double num, byte dec, int x, int y, char divider,
		int length, char filler) {
	char st[27];
	boolean neg = false;

	if (dec < 1)
		dec = 1;
	else if (dec > 5)
		dec = 5;

	if (num < 0)
		neg = true;
	_convert_float(st, num, length, dec);
	if (divider != '.') {
		for (uint16_t i = 0; i < sizeof(st); i++)
			if (st[i] == '.')
				st[i] = divider;
	}

	if (filler != ' ') {
		if (neg) {
			st[0] = '-';
			for (uint16_t i = 1; i < sizeof(st); i++)
				if ((st[i] == ' ') || (st[i] == '-'))
					st[i] = filler;
		} else {
			for (uint16_t i = 0; i < sizeof(st); i++)
				if (st[i] == ' ')
					st[i] = filler;
		}
	}

	print(st, x, y);
}

void UTFT::setFont(uint8_t* font) {
	cfont.font = font;
	cfont.x_size = fontbyte(0);
	cfont.y_size = fontbyte(1);
	cfont.offset = fontbyte(2);
	cfont.numchars = fontbyte(3);
}

uint8_t* UTFT::getFont() {
	return cfont.font;
}

uint8_t UTFT::getFontXsize() {
	return cfont.x_size;
}

uint8_t UTFT::getFontYsize() {
	return cfont.y_size;
}

void UTFT::drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int scale)
{
	unsigned int col;
	int tx, ty, tc, tsx, tsy;
	byte VH, VL;
	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	if (scale==1)
	{
		if (orient==PORTRAIT)
		{

			setXY(x, y, x+sx-1, y+sy-1);
			for (tc=0; tc<(sx*sy); tc++)
			{
				col=pgm_read_word(&data[tc]);
				VH = col>>8;
				VL = col & 0xff;
				writedata16_last((VH<<8)|VL);
			}
			setAddr(0, 0, disp_x_size, disp_y_size);
			writecommand_last(ILI9341_RAMWR); // write to RAM

		}
		else
		{
			for (ty=0; ty<sy; ty++)
			{
				setXY(x, y+ty, x+sx-1, y+ty);
				for (tx=sx-1; tx>=0; tx--)
				{
					col=pgm_read_word(&data[(ty*sx)+tx]);
					VH = col>>8;
					VL = col & 0xff;
					writedata16_last((VH<<8)|VL);
				}
			}
			setAddr(0, 0, disp_y_size, disp_x_size);
			writecommand_last(ILI9341_RAMWR); // write to RAM
		}
	}
	else
	{
		if (orient==PORTRAIT)
		{
			for (ty=0; ty<sy; ty++)
			{
				setAddr(x, y+(ty*scale), x+((sx*scale)-1), y+(ty*scale)+scale);
				writecommand_last(ILI9341_RAMWR); // write to RAM
				for (tsy=0; tsy<scale; tsy++)
				for (tx=0; tx<sx; tx++)
				{
					col=pgm_read_word(&data[(ty*sx)+tx]);
					for (tsx=0; tsx<scale; tsx++) {
						VH = col>>8;
						VL = col & 0xff;
						writedata16_last((VH<<8)|VL);
					}
				}
			}
			setAddr(0, 0, disp_x_size, disp_y_size);
			writecommand_last(ILI9341_RAMWR); // write to RAM

		}
		else
		{
			for (ty=0; ty<sy; ty++)
			{
				for (tsy=0; tsy<scale; tsy++)
				{
					setAddr(x, y+(ty*scale)+tsy, x+((sx*scale)-1), y+(ty*scale)+tsy);
					writecommand_last(ILI9341_RAMWR); // write to RAM
					for (tx=sx-1; tx>=0; tx--)
					{
						col=pgm_read_word(&data[(ty*sx)+tx]);
						for (tsx=0; tsx<scale; tsx++) {
							VH = col>>8;
							VL = col & 0xff;
							writedata16_last((VH<<8)|VL);
						}
					}
				}
			}
			setAddr(0, 0, disp_y_size, disp_x_size);
			writecommand_last(ILI9341_RAMWR); // write to RAM
		}
	}
	SPI.endTransaction();
}

void UTFT::drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int deg, int rox, int roy)
{
	unsigned int col;
	int tx, ty, newx, newy;
	double radian;
	radian=deg*0.0175;

	if (deg==0)
	drawBitmap(x, y, sx, sy, data);
	else
	{
		for (ty=0; ty<sy; ty++)
		for (tx=0; tx<sx; tx++)
		{
			col=pgm_read_word(&data[(ty*sx)+tx]);

			newx=x+rox+(((tx-rox)*cos(radian))-((ty-roy)*sin(radian)));
			newy=y+roy+(((ty-roy)*cos(radian))+((tx-rox)*sin(radian)));

			setXY(newx, newy, newx, newy);
			LCD_Write_DATA(col>>8,col & 0xff);
		}

	}
	clrXY();
}

void UTFT::lcdOff() {
	TFT_BL_OFF;
}

void UTFT::lcdOn() {
	TFT_BL_ON;
}

/*
 void UTFT::setContrast(char c)
 {

 }
 */

int UTFT::getDisplayXSize() {
	if (orient == PORTRAIT)
		return disp_x_size + 1;
	else
		return disp_y_size + 1;
}

int UTFT::getDisplayYSize() {
	if (orient == PORTRAIT)
		return disp_y_size + 1;
	else
		return disp_x_size + 1;
}

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void UTFT::setRotation(uint8_t m) {
	SPI.beginTransaction(SPISettings(SPICLOCK, MSBFIRST, SPI_MODE0));
	writecommand_cont(ILI9341_MADCTL);
	rotation = m % 4; // can't be higher than 3
	switch (rotation) {
	case 0:
		writedata8_last(MADCTL_MX | MADCTL_BGR);
		orient = LANDSCAPE;
		break;
	case 1:
		writedata8_last(MADCTL_MV | MADCTL_BGR);
		orient = PORTRAIT;
		break;
	case 2:
		writedata8_last(MADCTL_MY | MADCTL_BGR);
		orient = LANDSCAPE;
		break;
	case 3:
		writedata8_last(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
		orient = PORTRAIT;
		break;
	}
	SPI.endTransaction();
}
