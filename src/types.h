#ifndef TYPES_H
#define TYPES_H

#include <SDL2/SDL.h>

typedef struct _Camera {
    float x, y, z;
    float angle;
    float f;
} Camera;

struct _World;

typedef struct _Sprite {
    float x, y, z;
    int w, h;
    SDL_Texture* texture;

    struct _World* world;

    struct _Sprite* prev;
    struct _Sprite* next;
} Sprite;

typedef struct _World {
    SDL_Surface* srcImg;
    SDL_Texture* target;
    SDL_Surface* collision;
    int scale;

    Sprite head;
    int nSprites;
} World;

typedef struct _Kart {
    Sprite s;
    float rot;
    float speed;
    float driftTurnSpeed;
    struct {
        int isAccel : 1;
        int isDecel : 1;
        int turnR : 1;
        int turnL : 1;
        int driftR : 1;
        int driftL : 1;
    } flags;
    struct {
        float ACCELERATION;
        float MAX_SPEED;
        float TURN_SPEED;
        float TURN_MULT;
        float INIT_DRIFT_TURN;
        float MIN_DRIFT_TURN;
        float MAX_DRIFT_TURN;
        float FRICTION;
    } info;
} Kart;

typedef struct _KartFollowCam {
    Camera cam;
    Kart* kart;
    float followDist;
} KartFollowCam;

#endif