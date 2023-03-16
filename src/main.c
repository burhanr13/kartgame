#include "camera.h"
#include "kart.h"
#include "render_engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_PixelFormat* format = NULL;

Uint64 lastFrameTime = 0;
int frameCount = 0;
float elapsedTime = 0;
float fps;

World* world;

Camera* cam;

KartFollowCam* kCam;

Kart* kart;

SDL_Texture* kartTex;
SDL_Texture* objTex;

int quit = SDL_FALSE;

void init() {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    window = SDL_CreateWindow("Kart Game", SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                              SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);

    format = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);

    kartTex = IMG_LoadTexture(renderer, "resources/driver.png");
    objTex = IMG_LoadTexture(renderer, "resources/object.png");
}

void end() {

    SDL_DestroyTexture(kartTex);
    kartTex = NULL;
    SDL_DestroyTexture(objTex);
    objTex = NULL;
    destroyKart(kart);
    kart = NULL;
    free(kCam);
    kCam = NULL;

    while (world->head.next) {
        Sprite* s = world->head.next;
        unlinkSprite(s);
        free(s);
    }

    destroyWorld(world);
    world = NULL;

    SDL_DestroyRenderer(renderer);
    renderer = NULL;
    SDL_DestroyWindow(window);
    window = NULL;

    SDL_FreeFormat(format);
    format = NULL;

    IMG_Quit();
    SDL_Quit();
}

void makeSprites() {
    int pos[15][2] = {{1070, 1070}, {1070, 1240}, {1070, 1300}, {1150, 450},
                      {1260, 450},  {580, 1030},  {1290, 150},  {1500, 170},
                      {1920, 140},  {1950, 440},  {1770, 1220}, {1890, 1420},
                      {790, 1470},  {1940, 1250}, {1690, 980}};
    
    for (int i = 0; i < 15; i++) {
        Sprite* ptr = malloc(sizeof(Sprite));
        initSprite(ptr, objTex, pos[i][0], pos[i][1], 17, world);
    }
}

void handleEvent(SDL_Event* e) {
    switch (e->type) {
    case SDL_QUIT:
        quit = SDL_TRUE;
        break;
    }
}

void renderScene() {
    SDL_RenderClear(renderer);

    renderCourse(world, &kCam->cam);

    SDL_RenderPresent(renderer);
}

void updateTimeValues() {
    frameCount++;
    elapsedTime = (float) (SDL_GetTicks64() - lastFrameTime) / 1000;
    lastFrameTime = SDL_GetTicks64();
    fps = 1000 * (float) frameCount / SDL_GetTicks64();
}

int main(int argc, char* argv[]) {
    init();

    world = createWorld("resources/testcourse.png", "resources/testcourse.png",
                        1000);
    makeSprites();

    kart = createKart(100, 500, 0, kartTex, 10, world);
    kCam = createFollowCam(kart);

    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e)) {
            handleEvent(&e);
            kartHandleEvent(kart, &e);
        }

        updateKart(kart);
        updateFollowCamera(kCam);

        renderScene();

        updateTimeValues();
    }

    end();

    return 0;
}
