/*
* Methods to access the internal SD storage and the SPI flash chip
*/

/* Variables */
//Card
Sd2Card card;
uint32_t cardSizeBlocks;
uint16_t cardCapacityMB;
//Cache for SD block
cache_t cache;
//Fake disk geometry
uint8_t numberOfHeads;
uint8_t sectorsPerTrack;
//MBR information
uint8_t partType;
uint32_t relSector;
uint32_t partSize;
//FAT parameters
uint16_t reservedSectors;
uint8_t sectorsPerCluster;
uint32_t fatStart;
uint32_t fatSize;
uint32_t dataStart;
//Cconstants for file system structure
uint16_t const BU16 = 128;
uint16_t const BU32 = 8192;
//Strings needed in file system structures
char noName[] = "NO NAME    ";
char fat16str[] = "FAT16   ";
char fat32str[] = "FAT32   ";

/* Methods*/

/* Returns the free space on the card in KB */
uint32_t getSDSpace() {
	startAltClockline(true);
	uint32_t freeKB = sd.vol()->freeClusterCount();
	freeKB *= sd.vol()->blocksPerCluster() / 2;
	endAltClockline();
	return freeKB;
}

/* Returns the internal sd size in MB */
uint16_t getCardSize() {
	//Start Card
	startAltClockline();
	//Get card info
	cardSizeBlocks = card.cardSize();
	uint16_t cardSize = (cardSizeBlocks + 2047) / 2048;
	//End Card
	endAltClockline();
	return cardSize;
}

//Refresh free space of the internal space
void refreshFreeSpace() {
	sdInfo = String((int)(getSDSpace() / 1024)) + "/" + String((int)(getCardSize() - 1)) + " MB";
}

/* Timestamp set for SDFat */
void dateTime(uint16_t* date, uint16_t* time) {
	// return date using FAT_DATE macro to format fields
	*date = FAT_DATE(year(), month(), day());
	// return time using FAT_TIME macro to format fields
	*time = FAT_TIME(hour(), minute(), second());
}

/* Initializes the SD card */
void initSD() {
	//Storage info string
	sdInfo = " - / - MB";
	//Init the SD Card except for Early-Bird #1
	if (mlx90614Version == 1) {
		startAltClockline();
		//Check if the sd card works
		if (!sd.begin(pin_sd_cs, SPI_FULL_SPEED))
			setDiagnostic(diag_sd);
		else
			//Begin card
			card.begin(pin_sd_cs, SPI_FULL_SPEED);
		endAltClockline();
		//Calc free space
		refreshFreeSpace();
	}
	//Set SD Timestamp to current time
	SdFile::dateTimeCallback(dateTime);
}

/* Zero cache and optionally set the sector signature */
void clearCache(uint8_t addSig) {
	memset(&cache, 0, sizeof(cache));
	if (addSig) {
		cache.mbr.mbrSig0 = BOOTSIG0;
		cache.mbr.mbrSig1 = BOOTSIG1;
	}
}

/* Write cached block to the card */
uint8_t writeCache(uint32_t lbn) {
	return card.writeBlock(lbn, cache.data);
}

/* Return cylinder number for a logical block number */
uint16_t lbnToCylinder(uint32_t lbn) {
	return lbn / (numberOfHeads * sectorsPerTrack);
}

/* Return head number for a logical block number */
uint8_t lbnToHead(uint32_t lbn) {
	return (lbn % (numberOfHeads * sectorsPerTrack)) / sectorsPerTrack;
}

/* Return sector number for a logical block number */
uint8_t lbnToSector(uint32_t lbn) {
	return (lbn % sectorsPerTrack) + 1;
}

/* Format and write the Master Boot Record */
void writeMbr() {
	clearCache(true);
	part_t* p = cache.mbr.part;
	p->boot = 0;
	uint16_t c = lbnToCylinder(relSector);
	p->beginCylinderHigh = c >> 8;
	p->beginCylinderLow = c & 0XFF;
	p->beginHead = lbnToHead(relSector);
	p->beginSector = lbnToSector(relSector);
	p->type = partType;
	uint32_t endLbn = relSector + partSize - 1;
	c = lbnToCylinder(endLbn);
	if (c <= 1023) {
		p->endCylinderHigh = c >> 8;
		p->endCylinderLow = c & 0XFF;
		p->endHead = lbnToHead(endLbn);
		p->endSector = lbnToSector(endLbn);
	}
	else {
		// Too big flag, c = 1023, h = 254, s = 63
		p->endCylinderHigh = 3;
		p->endCylinderLow = 255;
		p->endHead = 254;
		p->endSector = 63;
	}
	p->firstSector = relSector;
	p->totalSectors = partSize;
	writeCache(0);
}

/* Zero FAT and root dir area on SD */
void clearFatDir(uint32_t bgn, uint32_t count) {
	clearCache(false);
	card.writeStart(bgn, count);
	for (uint32_t i = 0; i < count; i++) {
		card.writeData(cache.data);
	}
	card.writeStop();
}

/* Generate serial number from card size and micros since boot */
uint32_t volSerialNumber() {
	return (cardSizeBlocks << 8) + micros();
}

/* Format the internal storage with FAT16*/
void formatCard() {
	//Start Card
	startAltClockline();
	card.begin(pin_sd_cs, SPI_FULL_SPEED);
	//Get card info
	cardSizeBlocks = card.cardSize();
	cardCapacityMB = (cardSizeBlocks + 2047) / 2048;
	//Determine Sectors
	if (cardCapacityMB <= 16) {
		sectorsPerCluster = 2;
	}
	else if (cardCapacityMB <= 32) {
		sectorsPerCluster = 4;
	}
	else if (cardCapacityMB <= 64) {
		sectorsPerCluster = 8;
	}
	else if (cardCapacityMB <= 128) {
		sectorsPerCluster = 16;
	}
	else if (cardCapacityMB <= 1024) {
		sectorsPerCluster = 32;
	}
	else if (cardCapacityMB <= 32768) {
		sectorsPerCluster = 64;
	}
	else {
		// SDXC cards
		sectorsPerCluster = 128;
	}
	// set fake disk geometry
	sectorsPerTrack = cardCapacityMB <= 256 ? 32 : 63;
	//Determine Heads
	if (cardCapacityMB <= 16) {
		numberOfHeads = 2;
	}
	else if (cardCapacityMB <= 32) {
		numberOfHeads = 4;
	}
	else if (cardCapacityMB <= 128) {
		numberOfHeads = 8;
	}
	else if (cardCapacityMB <= 504) {
		numberOfHeads = 16;
	}
	else if (cardCapacityMB <= 1008) {
		numberOfHeads = 32;
	}
	else if (cardCapacityMB <= 2016) {
		numberOfHeads = 64;
	}
	else if (cardCapacityMB <= 4032) {
		numberOfHeads = 128;
	}
	else {
		numberOfHeads = 255;
	}
	//Make FAT16 filesystem
	uint32_t nc;
	for (dataStart = 2 * BU16;; dataStart += BU16) {
		nc = (cardSizeBlocks - dataStart) / sectorsPerCluster;
		fatSize = (nc + 2 + 255) / 256;
		uint32_t r = BU16 + 1 + 2 * fatSize + 32;
		if (dataStart < r) {
			continue;
		}
		relSector = dataStart - r + BU16;
		break;
	}
	reservedSectors = 1;
	fatStart = relSector + reservedSectors;
	partSize = nc * sectorsPerCluster + 2 * fatSize + reservedSectors + 32;
	if (partSize < 32680) {
		partType = 0X01;
	}
	else if (partSize < 65536) {
		partType = 0X04;
	}
	else {
		partType = 0X06;
	}
	// write MBR
	writeMbr();
	clearCache(true);
	fat_boot_t* pb = &cache.fbs;
	pb->jump[0] = 0XEB;
	pb->jump[1] = 0X00;
	pb->jump[2] = 0X90;
	for (uint8_t i = 0; i < sizeof(pb->oemId); i++) {
		pb->oemId[i] = ' ';
	}
	pb->bytesPerSector = 512;
	pb->sectorsPerCluster = sectorsPerCluster;
	pb->reservedSectorCount = reservedSectors;
	pb->fatCount = 2;
	pb->rootDirEntryCount = 512;
	pb->mediaType = 0XF8;
	pb->sectorsPerFat16 = fatSize;
	pb->sectorsPerTrack = sectorsPerTrack;
	pb->headCount = numberOfHeads;
	pb->hidddenSectors = relSector;
	pb->totalSectors32 = partSize;
	pb->driveNumber = 0X80;
	pb->bootSignature = EXTENDED_BOOT_SIG;
	pb->volumeSerialNumber = volSerialNumber();
	memcpy(pb->volumeLabel, noName, sizeof(pb->volumeLabel));
	memcpy(pb->fileSystemType, fat16str, sizeof(pb->fileSystemType));
	// write partition boot sector
	writeCache(relSector);
	// clear FAT and root directory
	clearFatDir(fatStart, dataStart - fatStart);
	clearCache(false);
	cache.fat16[0] = 0XFFF8;
	cache.fat16[1] = 0XFFFF;
	// write first block of FAT and backup for reserved clusters
	writeCache(fatStart);
	writeCache(fatStart + fatSize);
	//End SD
	endAltClockline();
}