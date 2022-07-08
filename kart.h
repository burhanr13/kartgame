#ifndef KART_H
#define KART_H

#include <sdl2/SDL.h>
#include "types.h"

Kart *createKart(double x, double y, double rot, SDL_Texture *tex, double w);
void updateKart(Kart *k);

#endif