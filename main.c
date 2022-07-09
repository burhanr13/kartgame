#include "camera.h"
#include "kart.h"
#include "render_engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_PixelFormat *format = NULL;

Uint64 lastFrameTime = 0;
int frameCount = 0;
Uint32 elapsedTime = 0;
float fps;

World *world;

Camera *cam;

KartFollowCam *kCam;

Kart *kart;

SDL_Texture *kartTex;

int quit = SDL_FALSE;

void init()
{
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    window = SDL_CreateWindow("Kart Game", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);

    format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);

    kartTex = IMG_LoadTexture(renderer, "resources/driver.png");
}

void close()
{

    SDL_DestroyWindow(window);
    window = NULL;
    SDL_DestroyRenderer(renderer);
    renderer = NULL;

    SDL_FreeFormat(format);
    format = NULL;

    SDL_DestroyTexture(kartTex);
    free(kart);
    free(kCam);

    SDL_DestroyTexture(world->sprites[1]->texture);

    for (int i = 1; i < world->nSprites; i++)
    {
        free(world->sprites[i]);
    }

    destroyWorld(world);
    world = NULL;

    SDL_Quit();
    IMG_Quit();
}

void makeSprites()
{
    world->sprites = malloc(15 * sizeof(Sprite *));
    world->nSprites = 15;
    SDL_Texture *tex = IMG_LoadTexture(renderer, "resources/object.png");
    int pos[15][2] = {{1070, 1070}, {1070, 1240}, {1070, 1300}, {1150, 450}, {1260, 450}, {580, 1030}, {1290, 150}, {1500, 170}, {1920, 140}, {1950, 440}, {1770, 1220}, {1890, 1420}, {790, 1470}, {1940, 1250}, {1690, 980}};
    int w;
    int h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h);
    Sprite *ptr;
    for (int i = 0; i < 15; i++)
    {
        ptr = malloc(sizeof(Sprite));
        initSprite(ptr, tex, pos[i][0], pos[i][1], 17);
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
    }
}

void renderScene()
{
    SDL_RenderClear(renderer);

    renderCourse(world, &kCam->cam);

    SDL_RenderPresent(renderer);
}

void updateTimeValues()
{
    frameCount++;
    elapsedTime = SDL_GetTicks64() - lastFrameTime;
    lastFrameTime = SDL_GetTicks64();
    fps = 1000 * (float)frameCount / SDL_GetTicks64();
}

int main(int argc, char *argv[])
{
    init();

    world = createWorld("resources/course.png", 0xFF76B0F5);
    makeSprites();

    kart = createKart(1000, 1000, 0, kartTex, 10);
    kCam = createFollowCam(kart);

    world->sprites[0] = &kart->s;

    SDL_Event e;

    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            handleEvent(e);
        }

        updateKart(kart);
        updateFollowCamera(kCam);

        renderScene();

        updateTimeValues();
        printf("%.0f fps\n", fps);
    }

    close();

    return 0;
}
