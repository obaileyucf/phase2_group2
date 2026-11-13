/**********************************************************************
 Copyright (c) 2020-2023, Unitree Robotics.Co.Ltd. All rights reserved.
***********************************************************************/
#ifdef COMPILE_WITH_MOVE_BASE

#ifndef STATE_MOVE_BASE_H
#define STATE_MOVE_BASE_H

#include "FSM/State_Trotting.h"
#include <geometry_msgs/msg/twist.hpp>
#include <rclcpp/rclcpp.hpp>

class State_move_base : public State_Trotting{
public:
    State_move_base(CtrlComponents *ctrlComp);
    ~State_move_base(){}
    FSMStateName checkChange();
private:
    void getUserCmd() override;
    void twistCallback(const geometry_msgs::msg::Twist &msg);
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr _cmdSub;
    rclcpp::Time _lastCmd{0, 0, RCL_ROS_TIME};
    double _vx = 0.0, _vy = 0.0, _wz = 0.0;
};

#endif  // STATE_MOVE_BASE_H

#endif  // COMPILE_WITH_MOVE_BASE