#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define RENDER_RES_W 512
#define RENDER_RES_H 512

typedef struct
{
    double x;
    double y;
    double angle;
    double fov;
    int minDist;
} Camera;

typedef struct
{
    double x;
    double y;
    int w;
    int h;
    SDL_Texture *texture;
} Sprite;

typedef struct
{
    SDL_Surface *srcImg;
    SDL_Texture *target;
    Uint32 color;
    Sprite **sprites;
    int nSprites;
} World;

typedef struct
{
    int move : 2;
    int turn : 2;
    int fov : 2;
    int minD : 2;
} InputState;

void init();
void close();
void loadImage();
void createDisplayTexture();
Camera *createCamera(int x, int y, double angle, double fov, int minDist);
World *createWorld(char *imgFile, Uint32 color);
void makeSprites();
void handleEvent(SDL_Event e);
void updateCamera();
void projectCameraViewOfSurfaceOntoTexture(SDL_Texture *target, int targetW, int targetH, SDL_Surface *src, Camera *cam);
void cameraToSurfaceCoord(Camera *cam, double u, double v, double *x, double *y);
Uint32 bilinear(double x, double y, Uint32 *pixels, int w);
void renderSprite(Sprite *o, double u, double v, Camera *cam);
void renderSprites(Sprite *sprites[], int nsprites, Camera *cam);
void renderScene();

#endif