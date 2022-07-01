#include <SDL2/SDL.h>
#include "render_engine.h"
#include "types.h"

Kart *createKart(double x,double y,double rot,SDL_Texture *tex,double w){
    Kart *kart = malloc(sizeof(Kart));
    initSprite(&kart->s, tex, x, y, w);
    kart->rot = rot;
    kart->speed = 0;
}

