#ifndef CAMERA_H
#define CAMERA_H

#include "types.h"

#define CAM_F 2
#define CAM_HEIGHT 20
#define CAM_FOLLOWDIST 70

Camera *createCamera(float x, float y, float z, float angle, float f);
KartFollowCam *createFollowCam(Kart *kart);
void updateFollowCamera(KartFollowCam *c);

#endif