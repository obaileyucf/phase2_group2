/**********************************************************************
 Copyright (c) 2020-2023, Unitree Robotics.Co.Ltd. All rights reserved.
***********************************************************************/
#include "interface/IOROS.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {
constexpr std::array<const char *, 12> jointNames{
    "FR_hip_joint", "FR_thigh_joint", "FR_calf_joint",
    "FL_hip_joint", "FL_thigh_joint", "FL_calf_joint",
    "RR_hip_joint", "RR_thigh_joint", "RR_calf_joint",
    "RL_hip_joint", "RL_thigh_joint", "RL_calf_joint"};
constexpr std::size_t fieldsPerJoint = 5;
}

IOROS::IOROS(const rclcpp::Node::SharedPtr &node)
    : _node(node), _keyboard(new KeyBoard()){
    cmdPanel = _keyboard;
    const auto feedbackQos = rclcpp::SensorDataQoS().keep_last(1);
    _joints = node->create_subscription<sensor_msgs::msg::JointState>(
        "/joint_states", feedbackQos,
        [this](const sensor_msgs::msg::JointState &msg) { jointCallback(msg); });
    _imu = node->create_subscription<sensor_msgs::msg::Imu>(
        "/trunk_imu", feedbackQos,
        [this](const sensor_msgs::msg::Imu &msg) { imuCallback(msg); });
    _motorCommands = node->create_publisher<std_msgs::msg::Float64MultiArray>(
        "/go1_effort_controller/commands", 1);
    _motorCommandMessage.data.resize(jointNames.size() * fieldsPerJoint);
}

bool IOROS::ready(const rclcpp::Time &now) const{
    constexpr double feedbackTimeout = 0.1;
    return now.nanoseconds() > 0 && _jointTime.nanoseconds() > 0 && _imuTime.nanoseconds() > 0
        && std::abs((now - _jointTime).seconds()) < feedbackTimeout
        && std::abs((now - _imuTime).seconds()) < feedbackTimeout;
}

void IOROS::sendZeroCommand(){
    std::fill(_motorCommandMessage.data.begin(), _motorCommandMessage.data.end(), 0.0);
    _motorCommands->publish(_motorCommandMessage);
}

void IOROS::sendRecv(const LowlevelCmd *cmd, LowlevelState *state){
    _keyboard->update();
    *state = _feedback;
    state->userCmd = cmdPanel->getUserCmd();
    state->userValue = cmdPanel->getUserValue();
    for(std::size_t i = 0; i < jointNames.size(); ++i){
        const auto &target = cmd->motorCmd[i];
        const std::array<double, fieldsPerJoint> fields = target.mode == 10
            ? std::array<double, fieldsPerJoint>{target.q, target.dq, target.Kp, target.Kd, target.tau}
            : std::array<double, fieldsPerJoint>{};
        std::copy(fields.begin(), fields.end(), _motorCommandMessage.data.begin() + i * fieldsPerJoint);
    }
    _motorCommands->publish(_motorCommandMessage);
}

void IOROS::jointCallback(const sensor_msgs::msg::JointState &msg){
    std::array<MotorState, jointNames.size()> joints{};
    for(std::size_t i = 0; i < jointNames.size(); ++i){
        const auto name = std::find(msg.name.begin(), msg.name.end(), jointNames[i]);
        const std::size_t index = std::distance(msg.name.begin(), name);
        if(name == msg.name.end() || index >= msg.position.size() || index >= msg.velocity.size()
                || !std::isfinite(msg.position[index]) || !std::isfinite(msg.velocity[index])){
            return;
        }
        joints[i].mode = 10;
        joints[i].q = msg.position[index];
        joints[i].dq = msg.velocity[index];
        if(index < msg.effort.size()){
            joints[i].tauEst = msg.effort[index];
        }
    }
    std::copy(joints.begin(), joints.end(), _feedback.motorState);
    _jointTime = _node->now();
}

void IOROS::imuCallback(const sensor_msgs::msg::Imu &msg){
    const auto &q = msg.orientation;
    const auto &a = msg.linear_acceleration;
    const auto &w = msg.angular_velocity;
    const double norm = std::sqrt(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
    if(!std::isfinite(norm) || norm < 1e-8
            || !std::isfinite(a.x) || !std::isfinite(a.y) || !std::isfinite(a.z)
            || !std::isfinite(w.x) || !std::isfinite(w.y) || !std::isfinite(w.z)){
        return;
    }
    _feedback.imu.quaternion[0] = q.w / norm;
    _feedback.imu.quaternion[1] = q.x / norm;
    _feedback.imu.quaternion[2] = q.y / norm;
    _feedback.imu.quaternion[3] = q.z / norm;
    _feedback.imu.accelerometer[0] = a.x;
    _feedback.imu.accelerometer[1] = a.y;
    _feedback.imu.accelerometer[2] = a.z;
    _feedback.imu.gyroscope[0] = w.x;
    _feedback.imu.gyroscope[1] = w.y;
    _feedback.imu.gyroscope[2] = w.z;
    _imuTime = _node->now();
}
