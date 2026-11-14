/**********************************************************************
 Copyright (c) 2020-2023, Unitree Robotics.Co.Ltd. All rights reserved.
***********************************************************************/
#ifndef IOROS_H
#define IOROS_H

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>

#include "interface/IOInterface.h"
#include "interface/KeyBoard.h"

class IOROS : public IOInterface{
public:
    explicit IOROS(const rclcpp::Node::SharedPtr &node);
    void sendRecv(const LowlevelCmd *cmd, LowlevelState *state) override;
    bool ready(const rclcpp::Time &now) const;
    void sendZeroCommand();

private:
    void jointCallback(const sensor_msgs::msg::JointState &msg);
    void imuCallback(const sensor_msgs::msg::Imu &msg);

    rclcpp::Node::SharedPtr _node;
    KeyBoard *_keyboard;
    LowlevelState _feedback{};
    rclcpp::Time _jointTime{0, 0, RCL_ROS_TIME};
    rclcpp::Time _imuTime{0, 0, RCL_ROS_TIME};
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr _joints;
    rclcpp::Subscription<sensor_msgs::msg::Imu>::SharedPtr _imu;
    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr _motorCommands;
    std_msgs::msg::Float64MultiArray _motorCommandMessage;
};

#endif  // IOROS_H
