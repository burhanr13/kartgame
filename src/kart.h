#ifndef KART_H
#define KART_H

#include "types.h"
#include <SDL2/SDL.h>

Kart *createKart(float x, float y, float rot, SDL_Texture *tex, int w);
void kartHandleEvent(Kart *k, SDL_Event *e);
void updateKart(Kart *k);

#endif