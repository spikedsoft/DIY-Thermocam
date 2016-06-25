/*
* Access the VC0706 camera module and display JPEGs
*/

/* Variables */

//JPEG Decompressor structure
typedef struct {
	const byte* jpic;
	unsigned short jsize;
	unsigned short joffset;
} IODEV;

//JPEG Decompressor
void* jdwork;
JDEC jd;
IODEV iodev;

//Combined mode for decompressor
bool combinedDecomp = false;

/* Methods */


/* Change the resolution of the device */
void changeCamRes(byte size) {
	//Change resolution
	cam.setImageSize(size);
	//Reset the device to change the resolution
	cam.reset();
	//Wait some time
	delay(300);
	//Re-establish the connection to the device
	cam.begin(115200);
	//Set camera compression
	cam.setCompression(95);
}

/* Init the camera module */
void initCamera() {
	//Start connection at 115.2k Baud
	cam.begin(115200);
	//Set camera compression
	cam.setCompression(95);
	//Test if the camera works
	if (!cam.takePicture()) {
		drawMessage((char*) "Visual camera is not working!");
		delay(1000);
		setDiagnostic(diag_camera);
	}
	//Skip the picture
	cam.end();
	//Check if the resolution is set to big
	if (cam.getImageSize() != VC0706_640x480)
		changeCamRes(VC0706_640x480);
}

/* Send the capture command to the camera*/
void captureVisualImage() {
	cam.takePicture();
}

/* Receive the visual image from the camera and save it on the SD card */
void saveVisualImage() {
	uint16_t jpglen = cam.frameLength();
	//Start alternative clockline
	startAltClockline();
	//Buffer to store the data
	uint8_t *buffer;
	while (jpglen > 0) {
		uint8_t bytesToRead = min(jpglen, 64);
		buffer = cam.readPicture(bytesToRead);
		sdFile.write(buffer, bytesToRead);
		jpglen -= bytesToRead;
	}
	sdFile.close();
	//End SD Transmission
	endAltClockline();
	//End camera
	cam.end();
}

/* Output function for the JPEG Decompressor - extracts the RGB565 values into the target array */
unsigned int output_func(JDEC * jd, void * bitmap, JRECT * rect) {
	//Help Variables
	byte redV, greenV, blueV, redT, greenT, blueT, red, green, blue;
	unsigned short pixel;
	unsigned short * bmp = (unsigned short *)bitmap;
	unsigned short x, y;
	unsigned short i = 0;
	//Go through the image
	for (y = rect->top; y <= rect->bottom; y++) {
		for (x = rect->left; x <= rect->right; x++) {
			//Write into the array with transparency if combined activated
			if (combinedDecomp) {
				//Get the visual image color
				pixel = bmp[i++];
				//And extract the RGB values out of it
				redV = (pixel & 0xF800) >> 8;
				greenV = (pixel & 0x7E0) >> 3;
				blueV = (pixel & 0x1F) << 3;
				//Get the thermal image color
				pixel = image[x + (y * 160)];
				//And extract the RGB values out of it
				redT = (pixel & 0xF800) >> 8;
				greenT = (pixel & 0x7E0) >> 3;
				blueT = (pixel & 0x1F) << 3;
				//Mix both
				red = redT * 0.5 + redV * 0.5;
				green = greenT * 0.5 + greenV * 0.5;
				blue = blueT * 0.5 + blueV * 0.5;
				//Set image to that calculated RGB value
				image[x + (y * 160)] = (((red & 248) | green >> 5) << 8)
					| ((green & 28) << 3 | blue >> 3);
			}
			//Write into the array if combined not activated
			else
				image[x + (y * 160)] = bmp[i++];
		}
	}
	return 1;
}

/* Help function to insert the JPEG data into the decompressor */
unsigned int input_func(JDEC * jd, byte* buff, unsigned int ndata) {
	IODEV * dev = (IODEV *)jd->device;
	ndata = (unsigned int)dev->jsize - dev->joffset > ndata ?
		ndata : dev->jsize - dev->joffset;
	if (buff)
		memcpy(buff, dev->jpic + dev->joffset, ndata);
	dev->joffset += ndata;
	return ndata;
}

/* Receive the image data and display it on the screen */
void getVisualImage() {
	//Get frame length
	uint16_t jpglen = cam.frameLength();
	//Define array for the jpeg data
	uint8_t* jpegData = (uint8_t*)calloc(jpglen, sizeof(uint8_t));
	//Buffer to store the data of up to 64 byte
	uint8_t *buffer = (uint8_t*)calloc(64, sizeof(uint8_t));
	//Count variable
	uint16_t counter = 0;
	//Transfer data
	uint16_t length = jpglen;
	while (length > 0) {
		uint8_t bytesToRead = min(length, 64);
		buffer = cam.readPicture(bytesToRead);
		for (int i = 0; i < bytesToRead; i++) {
			jpegData[counter] = buffer[i];
			counter++;
		}
		length -= bytesToRead;
	}
	//End transmission
	cam.end();
	//Decompress the image
	iodev.jpic = jpegData;
	iodev.jsize = jpglen;
	//the offset is zero
	iodev.joffset = 0;
	//Define space for the decompressor
	jdwork = malloc(3100);
	//Prepare the image for convertion to RGB565
	jd_prepare(&jd, input_func, jdwork, 3100, &iodev);
	//Small image, do not downsize
	if(cam.getImageSize() == VC0706_160x120)
		jd_decomp(&jd, output_func, 0);
	//320x240, do downsize to 160x120
	else
		jd_decomp(&jd, output_func, 1);
	//Free the jpeg data array
	free(jpegData);
	//Free space for the decompressor
	free(jdwork);
}