#include "render_engine.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <math.h>

extern SDL_Renderer* renderer;
extern SDL_PixelFormat* format;

const float RENDER_ASPECT_RATIO = (float) SCREEN_HEIGHT / SCREEN_WIDTH * 4 / 3;

World* createWorld(char* mapFile, char* collFile, int scale) {
    World* w = malloc(sizeof(World));

    SDL_Surface* s = IMG_Load(mapFile);
    w->srcImg = SDL_ConvertSurface(s, format, 0);
    SDL_FreeSurface(s);
    s = IMG_Load(collFile);
    w->collision = SDL_ConvertSurface(s, format, 0);
    SDL_FreeSurface(s);

    w->target = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                  SDL_TEXTUREACCESS_STREAMING, RENDER_RES_W,
                                  RENDER_RES_H);

    w->scale = scale;

    w->head.next = NULL;
    w->nSprites = 0;

    return w;
}

void destroyWorld(World* w) {
    SDL_FreeSurface(w->srcImg);
    SDL_DestroyTexture(w->target);
    free(w);
}

void initSprite(Sprite* s, SDL_Texture* tex, float x, float y, int w,
                World* world) {
    s->x = x;
    s->y = y;
    s->z = 0;
    s->texture = tex;
    SDL_QueryTexture(tex, NULL, NULL, &s->w, &s->h);
    s->h *= (float) w / s->w;
    s->w = w;

    s->world = world;
    world->nSprites++;

    if (world->head.next) world->head.next->prev = s;
    s->next = world->head.next;
    s->prev = &world->head;
    world->head.next = s;
}

void unlinkSprite(Sprite* s) {
    s->world->nSprites--;
    s->next->prev = s->prev;
    s->prev->next = s->next;
}

Uint32 interpColor(Uint32 c0, Uint32 c1, float t) {
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

Uint32 magFilter(float x, float y, Uint32* pixels, int w) {
    int pX = (int) x;
    int pY = (int) y;
    x -= pX;
    y -= pY;
    Uint32 col = pixels[pY * w + pX];
    Uint32 l = pixels[pY * w + pX - 1];
    Uint32 r = pixels[pY * w + pX + 1];
    Uint32 u = pixels[(pY - 1) * w + pX];
    Uint32 d = pixels[(pY + 1) * w + pX];
    Uint32 c2 = 0;

    if (x < 0.5) {
        col = interpColor(col, l, 0.5 - x);
        if (y < 0.5) {
            c2 = interpColor(u, pixels[(pY - 1) * w + pX - 1], 0.5 - x);
            col = interpColor(col, c2, 0.5 - y);
        } else {
            c2 = interpColor(d, pixels[(pY + 1) * w + pX - 1], 0.5 - x);
            col = interpColor(col, c2, y - 0.5);
        }
    } else {
        col = interpColor(col, r, x - 0.5);
        if (y < 0.5) {
            c2 = interpColor(u, pixels[(pY - 1) * w + pX + 1], x - 0.5);
            col = interpColor(col, c2, 0.5 - y);
        } else {
            c2 = interpColor(d, pixels[(pY + 1) * w + pX + 1], x - 0.5);
            col = interpColor(col, c2, y - 0.5);
        }
    }

    return col;
}

void cameraToSurfaceCoord(Camera* cam, float u, float v, float* x, float* y) {
    if (v == 0) return;
    float rotX = cam->z * (2 * u - 1) / (RENDER_ASPECT_RATIO * v);
    float rotY = cam->z * cam->f / v;
    *x = cam->x + rotX * cosf(cam->angle) + rotY * sinf(cam->angle);
    *y = cam->y + rotX * sinf(cam->angle) - rotY * cosf(cam->angle);
}

void surfaceToCameraCoord(Camera* cam, float x, float y, float* u, float* v) {
    float relX = x - cam->x;
    float relY = cam->y - y;
    float rotX = relX * cosf(cam->angle) - relY * sinf(cam->angle);
    float rotY = relX * sinf(cam->angle) + relY * cosf(cam->angle);
    if (rotY == 0) return;
    *u = (RENDER_ASPECT_RATIO * rotX * cam->f / rotY + 1) / 2;
    *v = cam->z * cam->f / rotY;
}

float calculateSpriteScale(Camera* cam, float v) {
    if (cam->z == 0) return 0;
    return SCREEN_WIDTH * RENDER_ASPECT_RATIO * v / (2 * cam->z);
}

void projectCameraViewOfSurfaceOntoTexture(SDL_Texture* target, int targetW,
                                           int targetH, SDL_Surface* src,
                                           Camera* cam, int scale) {
    void* pixelData = NULL;
    int pitch;
    SDL_LockTexture(target, NULL, &pixelData, &pitch);
    Uint32* pixels = (Uint32*) pixelData;
    SDL_LockSurface(src);
    Uint32* srcPixels = (Uint32*) src->pixels;

    float cosfa = cosf(cam->angle);
    float sinfa = sinf(cam->angle);

    for (int i = 0; i < targetH; i++) {

        float v = (float) (i + 1) / targetH;

        float rotXmod = cam->z / (RENDER_ASPECT_RATIO * v);
        float rotY = cam->z * cam->f / v;
        float xMod = cam->x + rotY * sinfa;
        float yMod = cam->y - rotY * cosfa;

        for (int j = 0; j < targetW; j++) {
            float u = (float) j / targetW;
            float rotX = rotXmod * (2 * u - 1);

            float x = xMod + rotX * cosfa;
            float y = yMod + rotX * sinfa;

            x /= scale;
            x *= src->w;
            y /= scale;
            y *= src->h;

            if (x < 1) x = 1;
            if (x >= src->w - 1) x = src->w - 2;
            if (y < 1) y = 1;
            if (y >= src->h - 1) y = src->h - 2;

            // pixels[i * targetW + j] = magFilter(x, y, srcPixels, src->w);
            pixels[i * targetW + j] = srcPixels[(int) y * src->w + (int) x];
        }
    }

    SDL_UnlockSurface(src);
    SDL_UnlockTexture(target);
}

void renderSprite(Sprite* o, float u, float v, Camera* cam) {
    float scale = calculateSpriteScale(cam, v);
    float scaledW = scale * o->w;
    float scaledH = scale * o->h;
    float scaledZ = scale * o->z;
    if (scaledW < 1 || scaledH < 1 || scaledW > SCREEN_WIDTH ||
        scaledH > SCREEN_HEIGHT)
        return;
    SDL_Rect r = {u * SCREEN_WIDTH - scaledW / 2,
                  ((1 + 2 * v) / 3) * SCREEN_HEIGHT - scaledH - scaledZ,
                  scaledW, scaledH};
    SDL_RenderCopy(renderer, o->texture, NULL, &r);
}

struct SpriteUV {
    Sprite* s;
    float u, v;
};

int cmpV(const void* a, const void* b) {
    struct SpriteUV m = *(struct SpriteUV*) a;
    struct SpriteUV n = *(struct SpriteUV*) b;
    return (m.v > n.v) ? 1 : ((m.v < n.v) ? -1 : 0);
}

void renderSprites(Sprite* sprites, int nsprites, Camera* cam) {
    struct SpriteUV inView[nsprites];
    int nInView = 0;

    for (Sprite* cur = sprites; cur; cur = cur->next) {
        float u = 0, v = 0;
        surfaceToCameraCoord(cam, cur->x, cur->y, &u, &v);
        if (-0.25 < u && u < 1.25 && v < 1.5) {
            inView[nInView].s = cur;
            inView[nInView].u = u;
            inView[nInView].v = v;
            nInView++;
        }
    }

    qsort(inView, nInView, sizeof(struct SpriteUV), cmpV);

    for (int i = 0; i < nInView; i++) {
        renderSprite(inView[i].s, inView[i].u, inView[i].v, cam);
    }
}

void renderCourse(World* w, Camera* cam) {
    projectCameraViewOfSurfaceOntoTexture(w->target, RENDER_RES_W, RENDER_RES_H,
                                          w->srcImg, cam, w->scale);
    SDL_Rect fieldRect = {0, SCREEN_HEIGHT / 3, SCREEN_WIDTH,
                          2 * SCREEN_HEIGHT / 3};
    SDL_RenderCopy(renderer, w->target, NULL, &fieldRect);
    renderSprites(w->head.next, w->nSprites, cam);
}
