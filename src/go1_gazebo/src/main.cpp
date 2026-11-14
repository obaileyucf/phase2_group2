
/**********************************************************************
 Copyright (c) 2020-2023, Unitree Robotics.Co.Ltd. All rights reserved.
**********************************************************************/

#include <chrono>
#include <memory>
#include <string>

#include <rclcpp/create_timer.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rcl_interfaces/msg/parameter_descriptor.hpp>

#include "Gait/GaitProfile.h"
#include "control/ControlFrame.h"
#include "interface/IOROS.h"

int main(int argc, char **argv)
{
    using namespace std::chrono_literals;

    rclcpp::init(argc, argv);

    auto node = std::make_shared<rclcpp::Node>(
        "unitree_gazebo_servo",
        rclcpp::NodeOptions().parameter_overrides({
            rclcpp::Parameter("use_sim_time", true)
        })
    );

    rcl_interfaces::msg::ParameterDescriptor gaitParameter;
    gaitParameter.description =
        "Available gaits: walk, trot, canter, gallop, gallop_rotary.";
    gaitParameter.read_only = true;

    const auto gait = node->declare_parameter<std::string>(
        "gait", "trot", gaitParameter
    );

    GaitProfile profile{};

    if (gait == "walk")
    {
        profile = walkProfile();
    }
    else if (gait == "trot")
    {
        profile = trotProfile();
    }
    else if (gait == "canter")
    {
        profile = canterProfile();
    }
    else if (gait == "gallop" || gait == "gallop_transverse")
    {
        profile = gallopProfile();
    }
    else if (gait == "gallop_rotary")
    {
        profile = gallopRotaryProfile();
    }
    else
    {
        RCLCPP_ERROR(
            node->get_logger(),
            "Unknown gait '%s'. Choose walk, trot, canter, gallop, or gallop_rotary.",
            gait.c_str()
        );

        rclcpp::shutdown();
        return 2;
    }

    auto *ioInter = new IOROS(node);

    CtrlComponents ctrlComp(ioInter, node, profile);

    ctrlComp.ctrlPlatform = CtrlPlatform::GAZEBO;

    constexpr auto controlPeriod = 2ms;
    ctrlComp.dt =
        std::chrono::duration<double>(controlPeriod).count();

    ctrlComp.robotModel = new Go1Robot();

    ctrlComp.waveGen = new WaveGenerator(
        profile.period,
        profile.dutyFactor,
        profile.touchdown
    );

    ctrlComp.geneObj();

    ControlFrame ctrlFrame(&ctrlComp);

    RCLCPP_INFO(
        node->get_logger(),
        "Gait: %s | 2: stand | 4: start gait | W/S: forward/back | Space: stop | 1: passive",
        profile.stateName
    );

    const auto timer = rclcpp::create_timer(
        node,
        node->get_clock(),
        controlPeriod,
        [&]()
        {
            const auto now = node->now();

            if (ioInter->ready(now))
            {
                ctrlFrame.run(now.seconds());
            }
            else
            {
                ioInter->sendZeroCommand();
            }
        }
    );

    rclcpp::spin(node);

    rclcpp::shutdown();

    return 0;
}
