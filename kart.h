#ifndef KART_H
#define KART_H

#include "types.h"
#include <sdl2/SDL.h>

Kart *createKart(float x, float y, float rot, SDL_Texture *tex, float w);
void updateKart(Kart *k);

#endif