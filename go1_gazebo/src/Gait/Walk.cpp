#include "Gait/GaitProfile.h"

GaitProfile walkProfile(){
    // Lateral-sequence crawl: FR, RL, FL, RR in time order.
    // The vector is indexed in Unitree motor order: FR, FL, RR, RL.
    return {1.10, 0.75, Vec4(0.0, 0.50, 0.75, 0.25), 0.05f, "walking", 0.10, 0.30};
}
