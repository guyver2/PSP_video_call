#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <psputility_netmodules.h>
#include <psputility_netparam.h>
#include <pspwlan.h>
#include <pspnet.h>
#include <pspnet_apctl.h>

#include <psppower.h>
#include <pspnet_inet.h>

#include <netinet/in.h>
#include <sys/select.h>

#include <stdio.h>
#include <errno.h>



#include "chotto/chotto.h"
#include "chotto/color.h"

#define printf pspDebugScreenPrintf

PSP_MODULE_INFO("test", 0, 1, 0);

/* Exit callback */
int exit_callback(int arg1, int arg2, void *common)
{
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

   thid = sceKernelCreateThread("update_thread", CallbackThread,
                 0x11, 0xFA0, PSP_THREAD_ATTR_USER, 0);
   if(thid >= 0)
   {
      sceKernelStartThread(thid, 0, 0);
   }

   return thid;
}
//-----------------------------------------------------------------------------

#define SERVER_PORT 7777

int make_socket(uint16_t port)
{
	int sock;
	int ret;
	struct sockaddr_in name;

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		return -1;
	}

	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	name.sin_addr.s_addr = /*inet_addr("193.169.1.6");//*/htonl(INADDR_ANY);
	//ret = connect(sock, (struct sockaddr *) &name, sizeof(name));
	ret = bind(sock, (struct sockaddr *) &name, sizeof(name));
	if(ret < 0)
	{
		return -1;
	}

	return sock;
}

/* Start a simple tcp echo server */
void start_server(const char *szIpAddr)
{
	int ret;
	int sock;
	int new = -1;
	struct sockaddr_in client;
	size_t size;
	int readbytes;
	char data[1024];
	fd_set set;
	fd_set setsave;
	/* Create a socket for listening */
	sock = make_socket(SERVER_PORT);
	if(sock < 0)
	{
		printf("Error creating server socket\n");
		return;
	}

	//---------------------------------------------------
	ret = listen(sock, 1);
	//ret = 0;
	if(ret < 0)
	{
		printf("Error calling listen\n");
		return;
	}

	printf("Listening for connections ip %s port %d\n", szIpAddr, SERVER_PORT);

	FD_ZERO(&set);
	FD_SET(sock, &set);
	setsave = set;

	while(1)
	{
		int i;
		set = setsave;
		if(select(FD_SETSIZE, &set, NULL, NULL, NULL) < 0)
		{
			printf("select error\n");
			return;
		}

		for(i = 0; i < FD_SETSIZE; i++)
		{
			if(FD_ISSET(i, &set))
			{
				int val = i;

				if(val == sock)
				{
					new = accept(sock, (struct sockaddr *) &client, &size);
					if(new < 0)
					{
						printf("Error in accept %s\n", strerror(errno));
						close(sock);
						return;
					}
					scePowerSetClockFrequency(333, 333, (333/2) ); // passe en 333Mhz
					printf("New connection %d from %s:%d\n", val, 
							inet_ntoa(client.sin_addr),
							ntohs(client.sin_port));

					//--------------------
					// envois les images de la cam
					while(1)
					{
					u32* tabChotto =  getTab();
					int pas = 4;
					//int i = 480*272*3;
					int limite = ((480*272)/(pas*pas));//*3;
					 write(new,&limite, sizeof(int));
					 u8 tmpEnvois[1024];
					 int i = 0;
					 int y = -pas;
					 //limite /=3;
					 int j = 0;
					for (i=0; i<limite; i++)
					 {
					  int x = (i*pas) % 480;
					  if (!x) y+=pas;
					  int addr = y*480+x;
					   u32 tmp = tabChotto[addr];
					   couleur c =rgba2coul(tmp);
					   //tabEnvois[j++] = c.r;
					   //tabEnvois[j++] = c.g;
					   //tabEnvois[j++] = c.b;
					   //write(new, &c.r, 1);
					   //write(new, &c.g, 1);
					   //write(new, &c.b, 1);
					   u8 octet = ((c.r >> 5)<<5) | ((c.g >> 5) << 2) | (c.b >> 6);
					   //write(new, &octet, 1);
					   tmpEnvois[j++] = octet;
					   if (j>=1000) { write(new, &tmpEnvois, 1000); j=0; fflush(NULL); }
					   //printf("(%d - %d - %d)\n", c.r, c.g, c.b);
					 }
					 write(new, &tmpEnvois, j);
					 fflush(NULL);
					 free(tabChotto);
					 u8 recep;
					 while(!read(new, &recep, 1)) ;
					 
					}
					//----------------------------- fin modifs
					FD_SET(new, &setsave);
				}
				else
				{
					readbytes = read(val, data, sizeof(data));
					if(readbytes <= 0)
					{
						printf("Socket %d closed\n", val);
						FD_CLR(val, &setsave);
						close(val);
					}
					else
					{
						write(val, data, readbytes);
						printf("%.*s", readbytes, data);
					}
				}
			}
		}
	}

	close(sock);
}
//-----------------------------------------------------------------------------

int connect_to_apctl(int config) {
  int err;
  int stateLast = -1;

  if (sceWlanGetSwitchState() != 1)
    pspDebugScreenClear();

  while (sceWlanGetSwitchState() != 1) {
    pspDebugScreenSetXY(0, 0);
    printf("Please enable WLAN to continue.\n");
    sceKernelDelayThread(1000 * 1000);
  }

  err = sceNetApctlConnect(config);
  if (err != 0) {
    printf("sceNetApctlConnect returns %08X\n", err);
    return 0;
  }

  printf("Connecting...\n");
  while (1) {
    int state;
    err = sceNetApctlGetState(&state);
    if (err != 0) {
      printf("sceNetApctlGetState returns $%x\n", err);
      break;
    }
    if (state != stateLast) {
      printf("  Connection state %d of 4.\n", state);
      stateLast = state;
    }
    if (state == 4) {
      break;
    }
    sceKernelDelayThread(50 * 1000);
  }
  printf("Connected!\n");
  sceKernelDelayThread(3000 * 1000);

  if (err != 0) {
    return 0;
  }

  return 1;
}

char *getconfname(int confnum) {
  static char confname[128];
  sceUtilityGetNetParam(confnum, PSP_NETPARAM_NAME, (netData *)confname);
  return confname;
}

int net_thread(SceSize args, void *argp)
{
  int selComponent = 1;
 
  printf("Using connection %d (%s) to connect...\n", selComponent, getconfname(selComponent));

  if (connect_to_apctl(selComponent))
  {
    char szMyIPAddr[32];
    if (sceNetApctlGetInfo(8, szMyIPAddr) != 0)
      strcpy(szMyIPAddr, "unknown IP address");
    printf("\n-->>  IP: %s  <<--\n\n", szMyIPAddr);
    // ---- init la chotto
    initChotto();
    start_server(szMyIPAddr);
  }

  
  
  return 0;
}

int InitialiseNetwork(void)
{
  int err;

  printf("load network modules...");
  err = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON);
  if (err != 0)
  {
    printf("Error, could not load PSP_NET_MODULE_COMMON %08X\n", err);
    return 1;
  }
  err = sceUtilityLoadNetModule(PSP_NET_MODULE_INET);
  if (err != 0)
  {
    printf("Error, could not load PSP_NET_MODULE_INET %08X\n", err);
    return 1;
  }
  printf("done\n");

  err = pspSdkInetInit();
  if (err != 0)
  {
    printf("Error, could not initialise the network %08X\n", err);
    return 1;
  }
  return 0;
}

/* Simple thread */
int main(int argc, char **argv)
{
   SceUID thid;

   SetupCallbacks();

   pspDebugScreenInit();

  if (InitialiseNetwork() != 0)
  {
    sceKernelSleepThread();
  }

  thid = sceKernelCreateThread("net_thread", net_thread, 0x18, 0x10000, PSP_THREAD_ATTR_USER, NULL);
  if (thid < 0) {
    printf("Error! Thread could not be created!\n");
    sceKernelSleepThread();
  }

  sceKernelStartThread(thid, 0, NULL);

  sceKernelExitDeleteThread(0);

   return 0;
}
