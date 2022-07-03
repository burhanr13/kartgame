#include <math.h>
#include <stdlib.h>

#include "camera.h"
#include "types.h"

KartFollowCam *createFollowCam(Kart *kart)
{
    KartFollowCam *c = malloc(sizeof(KartFollowCam));
    c->kart = kart;
    c->cam.f = CAM_F;
    c->cam.height = CAM_HEIGHT;
    c->followDist = CAM_FOLLOWDIST;
    updateFollowCamera(c);
}

void updateFollowCamera(KartFollowCam *c)
{
    c->cam.angle = c->kart->rot;
    c->cam.x = c->kart->s.x - c->followDist * sin(c->cam.angle);
    c->cam.y = c->kart->s.y + c->followDist * cos(c->cam.angle);
}