#include "render_engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

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

int main(int argc, char *argv[])
{
    init();

    // loadImage();
    // createDisplayTexture();
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
    // free(sprites);
    // sprites = NULL;

    free(world);
    world = NULL;

    SDL_Quit();
    IMG_Quit();
}

World *createWorld(char *imgFile, Uint32 color)
{
    World *w = malloc(sizeof(w));

    SDL_Surface *s = IMG_Load("course.png");
    w->srcImg = SDL_ConvertSurface(s, format, 0);
    SDL_FreeSurface(s);

    w->target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, RENDER_RES_W, RENDER_RES_H);
    w->color = color;

    return w;
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

void makeSprites()
{
    world->sprites = malloc(15 * sizeof(Sprite));
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
        ptr->x = pos[i][0];
        ptr->y = pos[i][1];
        ptr->w = 25;
        ptr->h = 25 * h / w;
        ptr->texture = tex;
        world->sprites[i] = ptr;
    }
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

Uint32 interpColor(Uint32 c0, Uint32 c1, double t)
{
    int a0, a1, r0, r1, g0, g1, b0, b1;
    a0 = (c0 >> 24) & 0xff;
    a1 = (c1 >> 24) & 0xff;
    r0 = (c0 >> 16) & 0xff;
    r1 = (c1 >> 16) & 0xff;
    g0 = (c0 >> 8) & 0xff;
    g1 = (c1 >> 8) & 0xff;
    b0 = (c0 >> 0) & 0xff;
    b1 = (c1 >> 0) & 0xff;

    int a = a0 + (a1 - a0) * t;
    int r = r0 + (r1 - r0) * t;
    int g = g0 + (g1 - g0) * t;
    int b = b0 + (b1 - b0) * t;

    return a << 24 | r << 16 | g << 8 | b << 0;
}

Uint32 magFilter(double x, double y, Uint32 *pixels, int w)
{
    int pX = (int)x;
    int pY = (int)y;
    x -= pX;
    y -= pY;
    Uint32 col = pixels[pY * w + pX];
    Uint32 l = pixels[pY * w + pX - 1];
    Uint32 r = pixels[pY * w + pX + 1];
    Uint32 u = pixels[(pY - 1) * w + pX];
    Uint32 d = pixels[(pY + 1) * w + pX];
    Uint32 c2 = 0;

    if (x < 0.5)
    {
        col = interpColor(col, l, 0.5 - x);
        if (y < 0.5)
        {
            c2 = interpColor(u, pixels[(pY - 1) * w + pX - 1], 0.5 - x);
            col = interpColor(col, c2, 0.5 - y);
        }
        else
        {
            c2 = interpColor(d, pixels[(pY + 1) * w + pX - 1], 0.5 - x);
            col = interpColor(col, c2, y - 0.5);
        }
    }
    else
    {
        col = interpColor(col, r, x - 0.5);
        if (y < 0.5)
        {
            c2 = interpColor(u, pixels[(pY - 1) * w + pX + 1], x - 0.5);
            col = interpColor(col, c2, 0.5 - y);
        }
        else
        {
            c2 = interpColor(d, pixels[(pY + 1) * w + pX + 1], x - 0.5);
            col = interpColor(col, c2, y - 0.5);
        }
    }

    return col;
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
    double x, y;

    for (int i = 0; i < targetW * targetH; i++)
    {
        u = (double)(i % targetW) / targetW;
        v = (double)(i / targetW) / targetH;
        cameraToSurfaceCoord(cam, u, v, &x, &y);
        if (x < 1 || y < 1 || x >= src->w - 1 || y >= src->h - 1)
        {
            pixels[i] = courseCol;
        }
        else
        {
            pixels[i] = magFilter(x, y, srcPixels, src->w);
        }
    }

    SDL_UnlockSurface(src);
    SDL_UnlockTexture(target);
}

void cameraToSurfaceCoord(Camera *cam, double u, double v, double *x, double *y)
{
    double vDist = cam->minDist / v;
    double uDist = tan(cam->fov / 2) * (2 * u - 1);
    *x = cam->x + vDist * (sin(cam->angle) + uDist * cos(cam->angle));
    *y = cam->y - vDist * (cos(cam->angle) - uDist * sin(cam->angle));
}

void surfaceToCameraCoord(Camera *cam, double x, double y, double *u, double *v)
{
    double relX = x - cam->x;
    double relY = cam->y - y;
    double twoUMinusOne = (relX * cos(cam->angle) - relY * sin(cam->angle)) /
                          (relY * cos(cam->angle) + relX * sin(cam->angle)) / tan(cam->fov / 2);
    *u = (twoUMinusOne + 1) / 2;
    *v = cam->minDist / relX * (sin(cam->angle) + tan(cam->fov / 2) * cos(cam->angle) * twoUMinusOne);
}

double calculateSpriteScale(int w, Camera *cam, double v)
{
    return SCREEN_WIDTH * v / (2 * cam->minDist * tan(cam->fov / 2));
}

void renderCourse(World *w, Camera *cam)
{
    projectCameraViewOfSurfaceOntoTexture(w->target, RENDER_RES_W, RENDER_RES_H, w->srcImg, cam);
    SDL_Rect fieldRect = {0, SCREEN_HEIGHT / 3, SCREEN_WIDTH, 2 * SCREEN_HEIGHT / 3};
    SDL_RenderCopy(renderer, w->target, NULL, &fieldRect);
    renderSprites(w->sprites, w->nSprites, cam);
}

void renderSprite(Sprite *o, double u, double v, Camera *cam)
{
    double scale = calculateSpriteScale(o->w, cam, v);
    double scaledW = scale * o->w;
    double scaledH = scale * o->h;
    SDL_Rect r = {u * SCREEN_WIDTH - scaledW / 2, ((1 + 2 * v) / 3) * SCREEN_HEIGHT - scaledH, scaledW, scaledH};
    SDL_RenderCopy(renderer, o->texture, NULL, &r);
}

void swap(double arr[], int a, int b)
{
    double temp = arr[a];
    arr[a] = arr[b];
    arr[b] = temp;
}

void sortByV(Sprite *sprites[], double us[], double vs[], int start, int end)
{
    if (start >= end)
        return;
    double pivot = vs[(start + end) / 2];
    int a = start;
    int b = end;
    while (a < b)
    {
        while (vs[a] < pivot)
        {
            a++;
        }
        while (vs[b] > pivot)
        {
            b--;
        }
        if (a > b)
            break;
        swap(us, a, b);
        swap(vs, a, b);
        Sprite *temp = sprites[a];
        sprites[a] = sprites[b];
        sprites[b] = temp;
        a++;
        b--;
    }
    sortByV(sprites, us, vs, start, b);
    sortByV(sprites, us, vs, a, end);
}

void renderSprites(Sprite *sprites[], int nsprites, Camera *cam)
{
    double us[nsprites];
    double vs[nsprites];
    double u, v;
    Sprite *inView[nsprites];
    int nInView = 0;
    int i;
    for (i = 0; i < nsprites; i++)
    {
        surfaceToCameraCoord(cam, sprites[i]->x, sprites[i]->y, &u, &v);
        if (-0.25 < u && u < 1.25 && v < 1.5)
        {
            inView[nInView] = sprites[i];
            us[nInView] = u;
            vs[nInView] = v;
            nInView++;
        }
    }
    sortByV(inView, us, vs, 0, nInView - 1);

    for (i = 0; i < nInView; i++)
    {
        renderSprite(inView[i], us[i], vs[i], cam);
    }
}

void renderScene()
{
    SDL_RenderClear(renderer);

    renderCourse(world, cam);

    SDL_RenderPresent(renderer);
}