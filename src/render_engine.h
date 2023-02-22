#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include "types.h"
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define RENDER_RES_W 512
#define RENDER_RES_H 512

World* createWorld(char* imgFile, Uint32 color);
void destroyWorld(World* w);
void initSprite(Sprite* s, SDL_Texture* tex, float x, float y, int w);
void renderCourse(World* world, Camera* camera);

#endif