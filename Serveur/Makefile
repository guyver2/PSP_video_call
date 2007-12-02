TARGET = Hermes-Serveur
OBJS = main.o chotto/chotto.o chotto/color.o

INCDIR = 
CFLAGS = -O3 -G0 -Wall -g -static
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION=271


LIBDIR =

LIBS = -lpspsdk -lpspctrl  -lpsprtc -lpsppower -lpspgu -lpspaudiolib -lpspaudio -lpsphprm\
	-lpspusb -lpsputility -lpspusbcam -lpspjpeg -lpspwlan -lpspnet

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_ICON = ICON0.png

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
