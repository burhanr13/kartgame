#include "render_engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

extern SDL_Renderer *renderer;
extern SDL_PixelFormat *format;

const double RENDER_ASPECT_RATIO = (double)SCREEN_HEIGHT / SCREEN_WIDTH * 2 / 3;

World *createWorld(char *imgFile, Uint32 color)
{
    World *w = malloc(sizeof(World));

    SDL_Surface *s = IMG_Load(imgFile);
    w->srcImg = SDL_ConvertSurface(s, format, 0);
    SDL_FreeSurface(s);

    w->target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, RENDER_RES_W, RENDER_RES_H);
    w->color = color;

    return w;
}

void initSprite(Sprite *s, SDL_Texture *tex, double x, double y, double w)
{
    s->x = x;
    s->y = y;
    s->texture = tex;
    SDL_QueryTexture(tex, NULL, NULL, &s->w, &s->h);
    s->h *= w / s->w;
    s->w = w;
}

Camera *createCamera(int x, int y, double angle, double f, int minDist)
{
    Camera *c = malloc(sizeof(*c));
    c->x = x;
    c->y = y;
    c->angle = angle;
    c->f = f;
    c->height = minDist;
    return c;
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

void cameraToSurfaceCoord(Camera *cam, double u, double v, double *x, double *y)
{
    if(v==0)
        return;
    double rotX = cam->height * (2 * u - 1) / (RENDER_ASPECT_RATIO * v);
    double rotY = cam->height * cam->f / v;
    *x = cam->x + rotX * cos(cam->angle) + rotY * sin(cam->angle);
    *y = cam->y + rotX * sin(cam->angle) - rotY * cos(cam->angle);
}

void surfaceToCameraCoord(Camera *cam, double x, double y, double *u, double *v)
{
    double relX = x - cam->x;
    double relY = cam->y - y;
    double rotX = relX * cos(cam->angle) - relY * sin(cam->angle);
    double rotY = relX * sin(cam->angle) + relY * cos(cam->angle);
    if(rotY==0)
        return;
    *u = (RENDER_ASPECT_RATIO * rotX * cam->f / rotY + 1) / 2;
    *v = cam->height * cam->f / rotY;
}

double calculateSpriteScale(Camera *cam, double v)
{
    if (cam->height == 0)
        return 0;
    return SCREEN_HEIGHT * v / (3 * cam->height);
}

void projectCameraViewOfSurfaceOntoTexture(SDL_Texture *target, int targetW, int targetH, SDL_Surface *src, Camera *cam, Uint32 color)
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
            pixels[i] = color;
        }
        else
        {
            pixels[i] = magFilter(x, y, srcPixels, src->w);
        }
    }

    SDL_UnlockSurface(src);
    SDL_UnlockTexture(target);
}

void renderSprite(Sprite *o, double u, double v, Camera *cam)
{
    double scale = calculateSpriteScale(cam, v);
    double scaledW = scale * o->w;
    double scaledH = scale * o->h;
    if(scaledW < 1 || scaledH < 1)
        return;
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

void renderCourse(World *w, Camera *cam)
{
    projectCameraViewOfSurfaceOntoTexture(w->target, RENDER_RES_W, RENDER_RES_H, w->srcImg, cam, w->color);
    SDL_Rect fieldRect = {0, SCREEN_HEIGHT / 3, SCREEN_WIDTH, 2 * SCREEN_HEIGHT / 3};
    SDL_RenderCopy(renderer, w->target, NULL, &fieldRect);
    renderSprites(w->sprites, w->nSprites, cam);
}