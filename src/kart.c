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
    kart->info.INIT_DRIFT_TURN = 1;
    kart->info.MIN_DRIFT_TURN = 0.1;
    kart->info.MAX_DRIFT_TURN = 2;
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
            // if (e->type == SDL_KEYUP && !k->flags.turnR)
            // {
            //     k->flags.driftL = k->flags.driftR = 0;
            // }
            break;
        case SDLK_d:
            k->flags.turnR = e->type == SDL_KEYDOWN;
            // if (e->type == SDL_KEYUP && !k->flags.turnL)
            // {
            //     k->flags.driftL = k->flags.driftR = 0;
            // }
            break;
        case SDLK_SPACE:
            if (e->type == SDL_KEYDOWN && !e->key.repeat)
            {
                k->flags.driftL = k->flags.turnL;
                k->flags.driftR = k->flags.turnR;
                k->rot -= (k->flags.driftR - k->flags.driftL) * M_PI_4;
                k->driftTurnSpeed = k->info.INIT_DRIFT_TURN;
                k->speed *= 0.75;
            }
            else if (e->type == SDL_KEYUP)
            {
                k->flags.driftL = k->flags.driftR = 0;
            }
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

void kartTranslateAngle(Kart *k, float angle, float distance)
{
    k->s.x += distance * sinf(k->rot + angle);
    k->s.y -= distance * cosf(k->rot + angle);
}

void updateKart(Kart *k)
{
    int driftDir = k->flags.driftR - k->flags.driftL;
    int turnDir = k->flags.turnR - k->flags.turnL;

    if (driftDir != 0)
    {
        k->driftTurnSpeed += driftDir * turnDir * 2 * elapsedTime;

        if (k->driftTurnSpeed > k->info.MAX_DRIFT_TURN)
            k->driftTurnSpeed = k->info.MAX_DRIFT_TURN;
        if (k->driftTurnSpeed < k->info.MIN_DRIFT_TURN)
            k->driftTurnSpeed = k->info.MIN_DRIFT_TURN;

        k->rot -= driftDir * k->driftTurnSpeed * elapsedTime;
        kartTranslateAngle(k, driftDir * M_PI_2, k->speed * elapsedTime);
        return;
    }

    k->rot -= turnDir * k->info.TURN_SPEED * elapsedTime;

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