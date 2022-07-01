#ifndef CAMERA_H
#define CAMERA_H

#include "types.h"

#define CAM_FOV 0.9
#define CAM_MINDIST 60
#define CAM_FOLLOWDIST 75

KartFollowCam *createFollowCam(Kart *kart);
void updateFollowCamera(KartFollowCam *c);

#endif