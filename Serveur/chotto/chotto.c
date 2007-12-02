
#include <pspkernel.h>
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
#include "color.h"
#include "chotto.h"


#define printf pspDebugScreenPrintf

void StopApp();

#define MAX_VIDEO_FRAME_SIZE	(32*1024)
#define MAX_STILL_IMAGE_SIZE	(512*1024)


//static u32 balleBuffer[480*272] __attribute__((aligned(64)));
static u32 shotBuffer[480*272] __attribute__((aligned(64)));
static u32 *tcpBuffer __attribute__((aligned(64)));
static u8  buffer[MAX_STILL_IMAGE_SIZE] __attribute__((aligned(64)));
static u8  work[68*1024] __attribute__((aligned(64)));
static u32 framebuffer[480*272] __attribute__((aligned(64)));
static u32  tampon[480*272] __attribute__((aligned(64)));

static SceUID waitphoto;



u32* getTab()
{
 int i;
 u32* res = malloc(480*272*sizeof(u32));
 for(i=0; i< 480*272; i++) res[i] = tcpBuffer[i];
 return res;
}

int video_thread(SceSize args, void *argp)
{
	PspUsbCamSetupVideoParam videoparam;
	int result;
	u32 *vram = (u32 *)0x04000000;

	memset(&videoparam, 0, sizeof(videoparam));
	videoparam.size = sizeof(videoparam);
	videoparam.resolution = PSP_USBCAM_RESOLUTION_480_272;
	videoparam.framerate = PSP_USBCAM_FRAMERATE_30_FPS;
	videoparam.wb = PSP_USBCAM_WB_AUTO;
	videoparam.saturation = 125;
	videoparam.brightness = 128;
	videoparam.contrast = 64;
	videoparam.sharpness = 0;
	videoparam.effectmode = PSP_USBCAM_EFFECTMODE_NORMAL;
	videoparam.framesize = MAX_VIDEO_FRAME_SIZE;
	videoparam.evlevel = PSP_USBCAM_EVLEVEL_0_0;	

	result = sceUsbCamSetupVideo(&videoparam, work, sizeof(work));	
	if (result < 0)
	{
		printf("Error 0x%08X in sceUsbCamSetupVideo.\n", result);
		sceKernelExitDeleteThread(result);
	}

	sceUsbCamAutoImageReverseSW(1);

	result = sceUsbCamStartVideo();	
	if (result < 0)
	{
		printf("Error 0x%08X in sceUsbCamStartVideo.\n", result);
		sceKernelExitDeleteThread(result);
	}

	sceDisplaySetMode(0, 480, 272);
	sceDisplaySetFrameBuf((void *)0x04000000, 512, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
	while(1)
	{
	int continuer = 1;
	while (continuer)
	{
		int i, j, m, n;
		// lit la video
		result = sceUsbCamReadVideoFrameBlocking(buffer, MAX_VIDEO_FRAME_SIZE);
		//FILE *fic = fopen("taille.txt", "a");
		//fprintf(fic,"taille : %d\n", result);
		//fclose(fic);
		if (result < 0)
		{
			printf("Error 0x%08X in sceUsbCamReadVideoFrameBlocking,\n", result);
			sceKernelExitDeleteThread(result);
		}

		result = sceJpegDecodeMJpeg(buffer, result, framebuffer, 0);
		if (result < 0)
		{
			printf("Error 0x%08X decoding mjpeg data.\n", result);
			sceKernelExitDeleteThread(result);
		}
		for (i = 0; i < 272; i++)
		{
			m = i*480;
			n = i*512;
			for (j = 0; j < 480; j++)
			{
 			 tampon[m+j] = framebuffer[m+j];
			}
		}
		//--------------------
		//flip l'affichage
		for (i = 0; i < 272; i++)
		{
		 m = i*480;
		 n = i*512;
		 for (j = 0; j < 480; j++)
		  {
		    tcpBuffer[m+j] = shotBuffer[m+j] = vram[n+j] = tampon[m+j];
		  }
		}
	}
	}
	sceKernelExitDeleteThread(0);
	return 0;	
}

#define TEMPO 0


void initChotto()
{
 SceUID thid;
 SceCtrlData pad;
 srand(time(0));
 printf("Hermes - Serveur.\n\n");
 printf("Homebrew from Guyver2, thanks to Kururin\n\n\n");
 printf("------transmit your IP adress to the client-----\n\nPlug the chotto cam ... and press [X]\n");
 //----------------------------------------------- modifications begining
 tcpBuffer = (u32*) malloc(480*272*sizeof(u32));
//------------------------------------------------ end of modifications
	while (1)
	{
		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS)
		{
			break;
		}

		sceKernelDelayThread(50000);
	}
 //--------------- chargement des modules de la camera
	if (LoadModules() < 0)
		sceKernelSleepThread();
	if (InitJpegDecoder() < 0)
		sceKernelSleepThread();
	
	if (StartUsb() < 0)
		sceKernelSleepThread();

	if (sceUsbActivate(PSP_USBCAM_PID) < 0)
	{
		printf("Error activating the camera.\n");
		sceKernelSleepThread();
	}

	while (1)
	{
		if ((sceUsbGetState() & 0xF) == PSP_USB_CONNECTION_ESTABLISHED)
			break;

		sceKernelDelayThread(50000);
	}

	waitphoto = sceKernelCreateSema("WaitPhotoSema", 0, 0, 1, NULL);
	if (waitphoto < 0)
	{
		printf("Cannot create semaphore.\n");
		sceKernelSleepThread();
	}

	thid = sceKernelCreateThread("video_thread", video_thread, 16, 256*1024, 0, NULL);
	if (thid < 0)
	{
		printf("Cannot create video thread.\n");
		sceKernelSleepThread();
	}

	if (sceKernelStartThread(thid, 0, NULL) < 0)
	{
		printf("Cannot start video thread.\n");
		sceKernelSleepThread();
	}
}



int LoadModules()
{
	int result = sceUtilityLoadUsbModule(PSP_USB_MODULE_ACC);
	if (result < 0)
	{
		printf("Error 0x%08X loading usbacc.prx.\n", result);
		return result;
	}

	result = sceUtilityLoadUsbModule(PSP_USB_MODULE_CAM);	
	if (result < 0)
	{
		printf("Error 0x%08X loading usbcam.prx.\n", result);
		return result;
	}

	// For jpeg decoding
	result = sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
	if (result < 0)
	{
		printf("Error 0x%08X loading avcodec.prx.\n", result);
	}

	return result;
}

int UnloadModules()
{
	int result = sceUtilityUnloadUsbModule(PSP_USB_MODULE_CAM);
	if (result < 0)
	{
		printf("Error 0x%08X unloading usbcam.prx.\n", result);
		return result;
	}

	result = sceUtilityUnloadUsbModule(PSP_USB_MODULE_ACC);
	if (result < 0)
	{
		printf("Error 0x%08X unloading usbacc.prx.\n", result);
		return result;
	}

	result = sceUtilityUnloadAvModule(PSP_AV_MODULE_AVCODEC);
	if (result < 0)
	{
		printf("Error 0x%08X unloading avcodec.prx.\n", result);
	}

	return result;
}

int StartUsb()
{
	int result = sceUsbStart(PSP_USBBUS_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X starting usbbus driver.\n", result);
		return result;
	}

	result = sceUsbStart(PSP_USBACC_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X starting usbacc driver.\n", result);
		return result;
	}
	
	result = sceUsbStart(PSP_USBCAM_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X starting usbcam driver.\n", result);
		return result;
	}

	result = sceUsbStart(PSP_USBCAMMIC_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X starting usbcammic driver.\n", result);		
	}

	return result;
}

int StopUsb()
{
	int result = sceUsbStop(PSP_USBCAMMIC_DRIVERNAME, 0, 0);	
	if (result < 0)
	{
		printf("Error 0x%08X stopping usbcammic driver.\n", result);
		return result;
	}
	
	result = sceUsbStop(PSP_USBCAM_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X stopping usbcam driver.\n", result);
		return result;
	}

	result = sceUsbStop(PSP_USBACC_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X stopping usbacc driver.\n", result);
		return result;
	}

	result = sceUsbStop(PSP_USBBUS_DRIVERNAME, 0, 0);
	if (result < 0)
	{
		printf("Error 0x%08X stopping usbbus driver.\n", result);
	}

	return result;
}

int InitJpegDecoder()
{
	int result = sceJpegInitMJpeg();
	if (result < 0)
	{
		printf("Error 0x%08X initing MJPEG library.\n", result);
		return result;
	}

	result = sceJpegCreateMJpeg(480, 272);
	if (result < 0)
	{
		printf("Error 0x%08X creating MJPEG decoder context.\n", result);
	}

	return result;
}

int FinishJpegDecoder()
{
	int result = sceJpegDeleteMJpeg();
	if (result < 0)
	{
		printf("Error 0x%08X deleting MJPEG decoder context.\n", result);
		return result;
	}

	result = sceJpegFinishMJpeg();
	if (result < 0)
	{
		printf("Error 0x%08X finishing MJPEG library.\n", result);
	}

	return result;
}


void StopApp()
{
	sceUsbDeactivate(PSP_USBCAM_PID);
	StopUsb();
	FinishJpegDecoder();
	UnloadModules();
}
