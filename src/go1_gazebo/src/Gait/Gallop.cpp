
#include "Gait/GaitProfile.h"

GaitProfile gallopProfile()
{
    GaitProfile profile;

    profile.period = 0.40;
    profile.dutyFactor = 0.35;

    // Transverse gallop:
    // RL -> RR -> FL -> FR.
    //
    // Ordering: [FR, FL, RR, RL].
    // The final part of the cycle has all four legs in swing.
    profile.touchdown << 0.60, 0.45, 0.20, 0.0;

    profile.swingHeight = 0.15;
    profile.stateName = "gallop_traverse";

    profile.longitudinalAccelerationLimit = 0.30;
    profile.backwardSpeedLimit = 0.15;

    return profile;
}

GaitProfile gallopRotaryProfile()
{
    GaitProfile profile;

    profile.period = 0.40;
    profile.dutyFactor = 0.35;

    // Rotary gallop:
    // FR -> FL -> RL -> RR.
    //
    // Ordering: [FR, FL, RR, RL].
    profile.touchdown << 0.0, 0.20, 0.45, 0.60;

    profile.swingHeight = 0.15;
    profile.stateName = "galloping_rotary";

    profile.longitudinalAccelerationLimit = 0.30;
    profile.backwardSpeedLimit = 0.15;

    return profile;
}
