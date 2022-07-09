#include "kart.h"
#include "render_engine.h"
#include "types.h"
#include <SDL2/SDL.h>
#include <math.h>

#define KART_SPEED 300
#define KART_TURN_SPEED 2

const extern Uint32 elapsedTime;

Kart *createKart(float x, float y, float rot, SDL_Texture *tex, float w)
{
    Kart *kart = malloc(sizeof(Kart));
    initSprite(&kart->s, tex, x, y, w);
    kart->rot = rot;
    kart->speed = 0;
    return kart;
}

void kartMove(Kart *k, float d)
{
    k->s.x += d * sinf(k->rot);
    k->s.y -= d * cosf(k->rot);
}

void updateKart(Kart *k)
{
    const Uint8 *keyStates = SDL_GetKeyboardState(NULL);

    k->rot += ((float)keyStates[SDL_SCANCODE_D] - keyStates[SDL_SCANCODE_A]) * KART_TURN_SPEED * elapsedTime / 1000;
    kartMove(k, ((float)keyStates[SDL_SCANCODE_W] - keyStates[SDL_SCANCODE_S]) * KART_SPEED * elapsedTime / 1000);
}