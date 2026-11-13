#ifndef GAITPROFILE_H
#define GAITPROFILE_H

#include "common/mathTypes.h"

struct GaitProfile {
    double period;
    double dutyFactor;
    Vec4 touchdown;
    float swingHeight;
    const char *stateName;
    double backwardSpeedLimit = 0.0;
    double longitudinalAccelerationLimit = 0.0;
};

GaitProfile walkProfile();
GaitProfile trotProfile();

#endif
