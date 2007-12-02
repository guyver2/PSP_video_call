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

#include <pspnet_inet.h>

#include <netinet/in.h>
#include <sys/select.h>

#include <stdio.h>
#include <errno.h>

#include "color.h"
#include "main.h"

#define printf pspDebugScreenPrintf

int sockfd;
struct sockaddr_in serv_addr;



void unInit()
{
close(sockfd);
}

void confirme()
{
 char tmp = 0;
 write(sockfd, &tmp, 1);
 fflush(NULL);
}


void recupImage(int t)
{
// --- recup des chaines
   long nbOctetsLus = 0;
   unsigned char tabSize[4] = {'\0', '\0', '\0', '\0'};
   int size = 0;
   confirme();
   while (nbOctetsLus<4) {
    int recut = 0;
    if ((recut = read(sockfd, tabSize+nbOctetsLus, 4))) {
     nbOctetsLus += recut;
    }
    else { printf("erreur de reception : recut = %d\n", recut); }
   }
   size = tabSize[0] | tabSize[1]<<8 | tabSize[2]<<16 | tabSize[3]<<24;
   //printf("on attend %d octets  ", size);
   //printf(" [%d %d %d %d] ", tabSize[0], tabSize[1], tabSize[2], tabSize[3]);
   nbOctetsLus = 0;
   while (nbOctetsLus<size) {
    int recut = 0;
    if ((recut = read(sockfd, &(tab[t][nbOctetsLus]), size-nbOctetsLus))) {
     nbOctetsLus += recut;
     //printf(" on en est a %d (%03d-%03d-%03d)\n", nbOctetsLus, tab[t][nbOctetsLus-3], tab[t][nbOctetsLus-2], tab[t][nbOctetsLus-1]);
    }
    else { printf("erreur de reception : recut = %d\n", recut); }
   }
   //printf("fin de recep\n\n");
}

//##############################################################################




#define SERVER_PORT 7777
int make_socket(uint16_t port, char* ip)
{

	int ret;
	struct sockaddr_in name;

	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		return -1;
	}

	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	//name.sin_addr.s_addr = htonl(ip);
	//name.sin_addr.s_un.S_un_b.s__b1 = (u8)193;
	sceNetInetInetAton(ip, &name.sin_addr);
	ret = connect(sockfd, (struct sockaddr *) &name, sizeof(name));
	//ret = bind(sock, (struct sockaddr *) &name, sizeof(name));
	if(ret < 0)
	{
		return -1;
	}

	return sockfd;
}

/* Start a simple tcp echo server */
void start_client(char *ip)
{
	/* Create a socket for listening */
	make_socket(SERVER_PORT, ip);
	if(sockfd < 0)
	{
		printf("Erreur de connection server socket\n");
		return;
	}
}








//------------------------- INITIALISATION

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

int init(int port, char* ip)
{
  InitialiseNetwork();
  int selComponent = 1;
 
  printf("Using connection %d (%s) to connect...\n", selComponent, getconfname(selComponent));

  if (connect_to_apctl(selComponent))
  {
    char szMyIPAddr[32];
    if (sceNetApctlGetInfo(8, szMyIPAddr) != 0)
      strcpy(szMyIPAddr, "unknown IP address");
    printf("IP: %s\n", szMyIPAddr);
    start_client(ip);
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






