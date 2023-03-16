#include <math.h>
#include <stdlib.h>

#include "camera.h"
#include "types.h"

Camera* createCamera(float x, float y, float z, float angle, float f) {
    Camera* c = malloc(sizeof(*c));
    c->x = x;
    c->y = y;
    c->z = z;
    c->angle = angle;
    c->f = f;
    return c;
}

KartFollowCam* createFollowCam(Kart* kart) {
    KartFollowCam* c = malloc(sizeof(KartFollowCam));
    c->kart = kart;
    c->cam.f = CAM_F;
    c->cam.z = CAM_HEIGHT;
    c->followDist = CAM_FOLLOWDIST;
    updateFollowCamera(c);
    return c;
}

void updateFollowCamera(KartFollowCam* c) {
    c->cam.angle = c->kart->rot;
    if (c->kart->flags.driftL) c->cam.angle += M_PI_4;
    if (c->kart->flags.driftR) c->cam.angle -= M_PI_4;
    c->cam.x = c->kart->s.x - c->followDist * sinf(c->cam.angle);
    c->cam.y = c->kart->s.y + c->followDist * cosf(c->cam.angle);
}