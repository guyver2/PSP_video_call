#include <stdio.h>
#include <stdlib.h>

#define CARRE(x) (x)*(x)
#include <psptypes.h>
#include "color.h"

couleur rgba2coul(u32 code)
 {
  couleur res;
  res.a = (code>>24);
  res.b = (code>>16);//&0x00ff;
  res.g = (code>>8);//&0x0000ff;
  res.r = code;//&0x000000ff;
  return res;
 }

u32 coul2rgba(couleur c)
 {
  return c.a<<24 | c.b<<16 | c.g<<8 | c.r;
 }


unsigned long distance(u32 c1, u32 c2)
 { //RGB is NOT the best color norme to find euclidian distance
  couleur coul1 = rgba2coul(c1);
  couleur coul2 = rgba2coul(c2);
  return CARRE(coul1.r - coul2.r)
       + CARRE(coul1.g - coul2.g)
       + 0.5*CARRE(coul1.b - coul2.b);
 }
