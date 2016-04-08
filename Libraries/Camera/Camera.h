/*
 * Camera.h
 *
 *  Created on: 18.11.2013
 *      Author: Max
 */

#ifndef CAMERA_H_
#define CAMERA_H_

#ifdef __cplusplus

#include "Arduino.h"

#define VC0706_640x480 0x00
#define VC0706_320x240 0x11
#define VC0706_160x120 0x22

class Camera{
 public:
  Camera(HardwareSerial *ser);
  boolean begin(uint32_t baud = 38400);
  boolean reset(void);
  boolean end(void);
  boolean changeBaudRate(void);
  boolean takePicture(void);
  uint8_t *readPicture(uint8_t n);
  uint32_t frameLength(void);
  uint8_t available();
  boolean setImageSize(uint8_t);
  uint8_t getImageSize();
  boolean setCompression(uint8_t c);
  boolean cameraFrameBuffCtrl(uint8_t command);

 private:
  uint8_t  serialNum;
  uint8_t  camerabuff[101];
  uint8_t  bufferLen;
  uint16_t frameptr;
  HardwareSerial *hwSerial;

  void common_init(void);

  boolean runCommand(uint8_t cmd, uint8_t *args, uint8_t argn,
	  uint8_t resplen, boolean flushflag = true);
  void sendCommand(uint8_t cmd, uint8_t args[], uint8_t argn);
  uint8_t readResponse(uint8_t numbytes, uint8_t timeout);
  boolean verifyResponse(uint8_t command);
};

#endif  // __cplusplus
#endif /* CAMERA_H_ */
