#include "kart.h"
#include "render_engine.h"
#include "types.h"
#include <SDL2/SDL.h>
#include <math.h>

#define EPSILON 0.01f

const extern float elapsedTime;

Kart *createKart(float x, float y, float rot, SDL_Texture *tex, int w)
{
    Kart *kart = malloc(sizeof(Kart));
    initSprite(&kart->s, tex, x, y, w);
    kart->rot = rot;
    kart->speed = 0;
    memset(&kart->flags, 0, sizeof(kart->flags));
    kart->info.MAX_SPEED = 300;
    kart->info.TURN_SPEED = 2;
    kart->info.ACCELERATION = 200;
    kart->info.TURN_MULT = 0.7;
    kart->info.FRICTION = 200;
    return kart;
}

void kartHandleEvent(Kart *k, SDL_Event *e)
{
    switch (e->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        switch (e->key.keysym.sym)
        {
        case SDLK_w:
            k->flags.isAccel = e->type == SDL_KEYDOWN;
            break;
        case SDLK_s:
            k->flags.isDecel = e->type == SDL_KEYDOWN;
            break;
        case SDLK_a:
            k->flags.turnL = e->type == SDL_KEYDOWN;
            break;
        case SDLK_d:
            k->flags.turnR = e->type == SDL_KEYDOWN;
            break;
        }
        break;
    }
}

void kartMove(Kart *k)
{
    float acSpeed = k->speed * (k->flags.turnL | k->flags.turnR ? k->info.TURN_MULT : 1) * elapsedTime;
    k->s.x += acSpeed * sinf(k->rot);
    k->s.y -= acSpeed * cosf(k->rot);
}

void updateKart(Kart *k)
{

    if (k->flags.turnL)
    {
        k->rot -= k->info.TURN_SPEED * elapsedTime;
    }
    else if (k->flags.turnR)
    {
        k->rot += k->info.TURN_SPEED * elapsedTime;
    }

    if (k->flags.isAccel)
    {
        k->speed += k->info.ACCELERATION * elapsedTime;
    }
    else if (k->flags.isDecel)
    {
        k->speed -= k->info.ACCELERATION * elapsedTime;
    }
    else
    {
        k->speed += k->info.FRICTION * elapsedTime * (k->speed > 0 ? -1 : (k->speed < 0 ? 1 : 0));
        if (fabsf(k->speed) < EPSILON)
        {
            k->speed = 0;
        }
    }

    if (k->speed > k->info.MAX_SPEED)
    {
        k->speed = k->info.MAX_SPEED;
    }
    else if (k->speed < -k->info.MAX_SPEED)
    {
        k->speed = -k->info.MAX_SPEED;
    }

    kartMove(k);
}