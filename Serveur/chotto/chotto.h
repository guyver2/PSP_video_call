#ifndef CHOTTO_H
#define CHOTTO_H
/*
#include <pspsdk.h>
#include <pspdebug.h>
#include <pspuser.h>
#include <pspdisplay.h>
#include <pspctrl.h>
#include <psputility_usbmodules.h>
#include <psputility_avmodules.h>
#include <pspusb.h>
#include <pspusbacc.h>
#include <pspusbcam.h>
#include <pspjpeg.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
*/
#include "color.h"
//int oldmain();

int video_thread(SceSize args, void *argp);
//int exit_callback(int arg1, int arg2, void *common);
//int CallbackThread(SceSize args, void *argp);
//int SetupCallbacks(void);
int LoadModules();
int UnloadModules();
int StartUsb();
int StopUsb();
int InitJpegDecoder();
int FinishJpegDecoder();
void StopApp();

void initChotto();
u32* getTab();

#endif
