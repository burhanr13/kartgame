#include "render_engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

typedef struct
{
    int move : 2;
    int turn : 2;
    int fov : 2;
    int minD : 2;
} InputState;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_PixelFormat *format = NULL;

SDL_Surface *source;
Uint32 courseCol = 0xFF76B0F5;
SDL_Texture *target;

World *world;

Camera *cam;
InputState input;

Sprite *sprites[15];

int quit = SDL_FALSE;

int debugCounter = 0;


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

    SDL_DestroyTexture(world->sprites[0]->texture);
    for (int i = 0; i < 15; i++)
    {
        free(world->sprites[i]);
    }

    free(world);
    world = NULL;

    SDL_Quit();
    IMG_Quit();
}

void makeSprites()
{
    world->sprites = malloc(15 * sizeof(Sprite *));
    world->nSprites = 15;
    SDL_Texture *tex = IMG_LoadTexture(renderer, "object.png");
    int pos[15][2] = {{1070, 1070}, {1070, 1240}, {1070, 1300}, {1150, 450}, {1260, 450}, {580, 1030}, {1290, 150}, {1500, 170}, {1920, 140}, {1950, 440}, {1770, 1220}, {1890, 1420}, {790, 1470}, {1940, 1250}, {1690, 980}};
    int w;
    int h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    Sprite *ptr;
    for (int i = 0; i < 15; i++)
    {
        ptr = malloc(sizeof(Sprite));
        initSprite(ptr, tex, pos[i][0], pos[i][1], 25);
        world->sprites[i] = ptr;
    }
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

void renderScene()
{
    SDL_RenderClear(renderer);

    renderCourse(world, cam);

    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
    init();

    world = createWorld("course.png", 0xFF76B0F5);
    cam = createCamera(1000, 1000, 0, 0.8, 50);
    makeSprites();

    SDL_Event e;

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
