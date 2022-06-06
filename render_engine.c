#include "render_engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_PixelFormat *format = NULL;

SDL_Surface *source;
SDL_Texture *target;

Camera *cam;
InputState input;

int quit = SDL_FALSE;

int debugCounter = 0;

int main(int argc, char *argv[])
{
    init();

    loadImage();
    createDisplayTexture();
    cam = createCamera(280, 500, 0, M_PI / 4, 50);

    SDL_Event e;
    printf("%d", sizeof(input));

    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            handleEvent(e);
        }
        updateCamera();

        renderScene();
        debugCounter++;
    }

    close();

    return 0;
}

void init()
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    window = SDL_CreateWindow("Projections", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);

    format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
}

void close()
{
    SDL_FreeSurface(source);
    source = NULL;
    SDL_DestroyTexture(target);
    target = NULL;

    SDL_DestroyWindow(window);
    window = NULL;
    SDL_DestroyRenderer(renderer);
    renderer = NULL;

    SDL_FreeFormat(format);
    format = NULL;

    SDL_Quit();
    IMG_Quit();
}

void loadImage()
{
    SDL_Surface *s = IMG_Load("course.png");
    source = SDL_ConvertSurface(s, format, 0);
    SDL_FreeSurface(s);
}

void createDisplayTexture()
{
    target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, RENDER_RES_W, RENDER_RES_H);
    SDL_SetTextureBlendMode(target, SDL_BLENDMODE_BLEND);
}

Camera *createCamera(int x, int y, double angle, double fov, int minDist)
{
    Camera *c = malloc(sizeof(*c));
    c->x = x;
    c->y = y;
    c->angle = angle;
    c->fov = fov;
    c->minDist = minDist;
    return c;
}

void handleEvent(SDL_Event e)
{
    switch (e.type)
    {
    case SDL_QUIT:
        quit = SDL_TRUE;
        break;
    case SDL_KEYUP:
        switch (e.key.keysym.sym)
        {
        case SDLK_w:
        case SDLK_s:
            input.move = 0;
            break;
        case SDLK_a:
        case SDLK_d:
            input.turn = 0;
            break;
        case SDLK_UP:
        case SDLK_DOWN:
            input.minD = 0;
            break;
        case SDLK_LEFT:
        case SDLK_RIGHT:
            input.fov = 0;
            break;
        }
        break;
    case SDL_KEYDOWN:
        switch (e.key.keysym.sym)
        {
        case SDLK_w:
            input.move = 1;
            break;
        case SDLK_s:
            input.move = -1;
            break;
        case SDLK_a:
            input.turn = -1;
            break;
        case SDLK_d:
            input.turn = 1;
            break;
        case SDLK_UP:
            input.minD = 1;
            break;
        case SDLK_DOWN:
            input.minD = -1;
            break;
        case SDLK_LEFT:
            input.fov = -1;
            break;
        case SDLK_RIGHT:
            input.fov = 1;
            break;
        }
        break;
    }
}

void updateCamera()
{

    cam->angle += input.turn * 0.01;
    cam->x += sin(cam->angle) * input.move * 3;
    cam->y -= cos(cam->angle) * input.move * 3;

    cam->fov += input.fov * 0.01;
    if (cam->fov < 0)
        cam->fov = 0;
    if (cam->fov > M_PI)
        cam->fov = M_PI;
    cam->minDist += input.minD * 2;
    if (cam->minDist < 0)
        cam->minDist = 0;
}

void projectCameraViewOfSurfaceOntoTexture(SDL_Texture *target, int targetW, int targetH, SDL_Surface *src, Camera *cam)
{
    void *pixelData = NULL;
    int pitch;
    SDL_LockTexture(target, NULL, &pixelData, &pitch);
    Uint32 *pixels = (Uint32 *)pixelData;
    SDL_LockSurface(src);
    Uint32 *srcPixels = (Uint32 *)src->pixels;

    double u, v;
    int x, y;

    for (int i = 0; i < targetW * targetH; i++)
    {
        u = (double)(i % targetW) / targetW;
        v = (double)(i / targetW) / targetH;
        cameraToSurfaceCoord(cam, u, v, &x, &y);
        if (x < 0 || y < 0 || x >= src->w || y >= src->h)
        {
            pixels[i] = SDL_MapRGBA(format, 0, 0, 0, 0);
        }
        else
        {
            pixels[i] = srcPixels[x + y * src->w];
        }
    }

    SDL_UnlockSurface(src);
    SDL_UnlockTexture(target);
}

void cameraToSurfaceCoord(Camera *cam, double u, double v, int *x, int *y)
{
    double vDist = cam->minDist / v;
    double uDist = tan(cam->fov / 2) * (2 * u - 1);
    *x = cam->x + vDist * (sin(cam->angle) + uDist * cos(cam->angle)) + 0.5;
    *y = cam->y - vDist * (cos(cam->angle) - uDist * sin(cam->angle)) + 0.5;
}

void surfaceToCameraCoord(Camera *cam, double x, double y, double *u, double *v)
{
    double twoUMinusOne = (x * cos(cam->angle) - y * sin(cam->angle)) /
                          (y * cos(cam->angle) + x * sin(cam->angle)) / tan(cam->fov);
    *u = (twoUMinusOne + 1) / 2;
    *v = cam->minDist / x * (sin(cam->angle) + tan(cam->fov / 2) * cos(cam->angle));
}

double calculateObjectScale(int w, Camera *cam, double v)
{
    return w * SCREEN_WIDTH * v / (2 * cam->minDist * tan(cam->fov / 2));
}

void renderScene()
{
    SDL_RenderClear(renderer);

    projectCameraViewOfSurfaceOntoTexture(target, RENDER_RES_W, RENDER_RES_H, source, cam);
    SDL_Rect fieldRect = {0, SCREEN_HEIGHT / 3, SCREEN_WIDTH, 2 * SCREEN_HEIGHT / 3};
    SDL_RenderCopy(renderer, target, NULL, &fieldRect);

    SDL_RenderPresent(renderer);
}