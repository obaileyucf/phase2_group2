
#include "Gait/GaitProfile.h"

GaitProfile canterProfile()
{
    GaitProfile profile;

    profile.period = 0.70;
    profile.dutyFactor = 0.55;

    // Three-beat sequence:
    // RL -> (RR + FL) -> FR.
    //
    // Ordering: [FR, FL, RR, RL].
    profile.touchdown << 0.30, 0.15, 0.15, 0.0;

    profile.swingHeight = 0.055;
    profile.stateName = "cantering";

    profile.longitudinalAccelerationLimit = 0.18;
    profile.backwardSpeedLimit = 0.15;

    return profile;
}
