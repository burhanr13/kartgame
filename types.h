#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>

typedef struct camera_s
{
    double x, y;
    double angle;
    double f;
    double height;
} Camera;

typedef struct sprite_s
{
    double x, y;
    int w, h;
    SDL_Texture *texture;
} Sprite;

typedef struct world_s
{
    SDL_Surface *srcImg;
    SDL_Texture *target;
    Uint32 color;
    Sprite **sprites;
    int nSprites;
} World;

typedef struct kart_s
{
    Sprite s;
    double rot;
    double speed;
} Kart;

typedef struct followcam_s
{
    Camera cam;
    Kart *kart;
    double followDist;
} KartFollowCam;

#endif