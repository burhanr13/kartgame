#ifndef CAMERA_H
#define CAMERA_H

#include "types.h"

#define CAM_F 1.7
#define CAM_HEIGHT 20
#define CAM_FOLLOWDIST 20

KartFollowCam *createFollowCam(Kart *kart);
void updateFollowCamera(KartFollowCam *c);

#endif