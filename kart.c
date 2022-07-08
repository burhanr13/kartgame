#include <SDL2/SDL.h>
#include <math.h>
#include "kart.h"
#include "render_engine.h"
#include "types.h"

#define KART_SPEED 0.4
#define KART_TURN_SPEED 0.002

const extern Uint32 elapsedTime;

Kart *createKart(double x,double y,double rot,SDL_Texture *tex,double w){
    Kart *kart = malloc(sizeof(Kart));
    initSprite(&kart->s, tex, x, y, w);
    kart->rot = rot;
    kart->speed = 0;
    return kart;
}

void kartMove(Kart *k, double d){
    k->s.x += d * sin(k->rot);
    k->s.y -= d * cos(k->rot);
}

void updateKart(Kart *k){
    const Uint8 *keyStates = SDL_GetKeyboardState(NULL);

    k->rot += (keyStates[SDL_SCANCODE_D] - keyStates[SDL_SCANCODE_A]) * KART_TURN_SPEED * elapsedTime;
    kartMove(k, (keyStates[SDL_SCANCODE_W] - keyStates[SDL_SCANCODE_S]) * KART_SPEED * elapsedTime);
}