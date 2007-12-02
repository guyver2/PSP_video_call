#ifndef COLOR_H
#define COLOR_H

#include <psptypes.h>


typedef struct {
 u8 r,g,b,a;
} couleur;

couleur rgba2coul(u32 code);
u32 coul2rgba(couleur c);
unsigned long distance(u32 c1, u32 c2);

#endif
