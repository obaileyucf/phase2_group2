
#include "Gait/GaitProfile.h"

GaitProfile canterProfile()
{
    GaitProfile profile;

    profile.period = 0.40;
    profile.dutyFactor = 0.45;

    // Three-beat sequence:
    // RL -> (RR + FL) -> FR.
    //
    // Ordering: [FR, FL, RR, RL].
    profile.touchdown << 0.50, 0.25, 0.25, 0.0;

    profile.swingHeight = 0.08;
    profile.stateName = "cantering";

    profile.longitudinalAccelerationLimit = 0.25;
    profile.backwardSpeedLimit = 0.15;

    return profile;
}
