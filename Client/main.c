#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <psputility_netmodules.h>
#include <psputility_netparam.h>
#include <pspwlan.h>
#include <pspnet.h>
#include <pspdisplay.h>
#include <pspnet_apctl.h>
#include <psppower.h>

#include <pspnet_inet.h>

#include <netinet/in.h>
#include <sys/select.h>

#include <stdio.h>
#include <errno.h>

#include "color.h"
#include "client.h"
#include "main.h"

#define printf pspDebugScreenPrintf



PSP_MODULE_INFO("test", 0, 1, 0);

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
   unInit();
   sceKernelExitGame();
   return 0;
}

/* Callback thread */
int CallbackThread(SceSize args, void *argp)
{
   int cbid;

   cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
   sceKernelRegisterExitCallback(cbid);
   sceKernelSleepThreadCB();

   return 0;
}

/* Sets up the callback thread and returns its thread id */
int SetupCallbacks(void)
{
   int thid = 0;

   thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
   if(thid >= 0)
   {
      sceKernelStartThread(thid, 0, 0);
   }

   return thid;
}







//--------------------------------------------------------------


int tabcourant = 0;
u32 buffer[480][272];
u8 **tab;
int pas = 4;



/******************************/
/*                            */
/*        mon exit            */
/*                            */
/******************************/


void monExit()
{
 unInit();
 exit(0);
}

void affichage()
{
 u32 *vram = (u32 *)0x04000000;
 //flip l'affichage
 int i,j,n;
 for (i = 0; i < 272; i++)
  {
   n = i*512;
   for (j = 0; j < 480; j++)
    {
     vram[n+j] = buffer[j][i];
    }
  }
}


void affPx(u8 r, u8 g, u8 b, int x, int y)
{
 int i, j;
 for(i=0; i<pas; i++)
  for(j=0; j<pas; j++)
 buffer[x+i][y+j] = 0x00000000 <<24 |  b<<16 | g<<8 | r;
 /*SDL_Rect rect;
 rect.w = pas;
 rect.h = pas;
 rect.x = x;
 rect.y = y;
 SDL_FillRect(fond, &rect, SDL_MapRGBA(fond->format,r,g,b,0));*/
}


void affImage()
{
 recupImage(tabcourant);
 int t = tabcourant;
 tabcourant++;
 tabcourant %= 2;
 int i;
 int j = 0;
 int limite = ((480*272) / (pas*pas)) ;//*3;
 int y = -pas;
 for(i=0; i<limite; i++/*=3*/)
  {
   int x = (j*pas) % 480;
   if (!x) y+=pas;
   j++;
   char r = ((tab[t][i])>>5) << 5 + 32;
   char g = ((tab[t][i])>>2) << 5 + 32;
   char b = ((tab[t][i])) << 6 + 64;
   affPx(r, g, b, x, y);
  }
 affichage();
}


int boucle(SceSize args, void *argp)
{
 sceDisplaySetMode(0, 480, 272);
 sceDisplaySetFrameBuf((void *)0x04000000, 512, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
while(1)
 {
  affImage();
 }
 return 0;
}


int main(int argc, char **argv)
{
 SceUID thid;
 SetupCallbacks();
 pspDebugScreenInit();
 char ip[32];
 u8 tabIP[4];
 // l'adresse suivante est celle de rosie sur mon reseau local
 // rosie est le petit nom de ma psp rose sur laquelle tourne la partie serveur
 tabIP[0] = 192;
 tabIP[1] = 168;
 tabIP[2] = 1;
 tabIP[3] = 51;
 int modif = 0;
 SceCtrlData pad, oldpad;
 int pret = 0;
 sceCtrlReadBufferPositive(&oldpad, 1);
 int rafraichir = 1;
 while(!pret)
 {
  if (rafraichir)
  {
   pspDebugScreenClear();
   printf("Hermes - Client\n\n");
   printf("Homebrew by Guyver2, thanks to Kururin\n\n\n");
   printf("------Enter server IP-----\n\n");
   printf("    %d.%d.%d.%d\n", tabIP[0], tabIP[1], tabIP[2], tabIP[3]);
   printf("change with arrows and valid with [X]\n");
   rafraichir = 0;
  }
  sceCtrlReadBufferPositive(&pad, 1);
  if (pad.Buttons == oldpad.Buttons) continue;
  if (pad.Buttons & PSP_CTRL_UP)
   {
    tabIP[modif]++;
     rafraichir = 1;
   }
   // fleches -> change l'ip
   if (pad.Buttons & PSP_CTRL_DOWN)
   {
    tabIP[modif]-=10;
     rafraichir = 1;
   }
   if (pad.Buttons & PSP_CTRL_LEFT)
   {
    modif--;
     rafraichir = 1;
    if (modif<0) modif = 0;
   }
   if (pad.Buttons & PSP_CTRL_RIGHT)
   {
    modif++;
     rafraichir = 1;
    if (modif>3) modif = 3;
   }
   // bouton X -> connexion
  if (pad.Buttons & PSP_CTRL_CROSS)
   {
    sprintf(ip, "%d.%d.%d.%d", tabIP[0], tabIP[1], tabIP[2], tabIP[3]);
    pret = 1;
     rafraichir = 1;
   }
  oldpad.Buttons = pad.Buttons;
  //sceKernelDelayThread(50000);
 }
 init(7777, ip);
 scePowerSetClockFrequency(333, 333, (333/2) ); // passe en 333Mhz
 tab = (u8**) malloc(2*sizeof(u8*));
 tab[0] = (u8*) malloc(480*272*3*sizeof(u8));
 tab[1] = (u8*) malloc(480*272*3*sizeof(u8));

 thid = sceKernelCreateThread("net_thread", boucle, 0x18, 0x10000, PSP_THREAD_ATTR_USER, NULL);
  if (thid < 0) {
    printf("Error! Thread could not be created!\n");
    sceKernelSleepThread();
  }

  sceKernelStartThread(thid, 0, NULL);

  sceKernelExitDeleteThread(0);

   return 0;

 
return 0;
}
