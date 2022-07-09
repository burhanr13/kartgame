#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>

typedef struct camera_s
{
    float x, y, z;
    float angle;
    float f;
} Camera;

typedef struct sprite_s
{
    float x, y, z;
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
    float rot;
    float speed;
} Kart;

typedef struct followcam_s
{
    Camera cam;
    Kart *kart;
    float followDist;
} KartFollowCam;

#endif