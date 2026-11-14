/**********************************************************************
 Copyright (c) 2020-2023, Unitree Robotics.Co.Ltd. All rights reserved.
***********************************************************************/
#ifdef COMPILE_WITH_MOVE_BASE

#include "FSM/State_move_base.h"
#include <cmath>

State_move_base::State_move_base(CtrlComponents *ctrlComp)
    :State_Trotting(ctrlComp){
    _stateName = FSMStateName::MOVE_BASE;
    _stateNameString = "move_base";
    _cmdSub = _ctrlComp->node->create_subscription<geometry_msgs::msg::Twist>(
        "/cmd_vel", 1, [this](const geometry_msgs::msg::Twist &msg) { twistCallback(msg); });
}

FSMStateName State_move_base::checkChange(){
    if(_lowState->userCmd == UserCommand::L2_B){
        return FSMStateName::PASSIVE;
    }
    else if(_lowState->userCmd == UserCommand::L2_A){
        return FSMStateName::FIXEDSTAND;
    }
    else{
        return FSMStateName::MOVE_BASE;
    }
}

void State_move_base::getUserCmd(){
    const double age = (_ctrlComp->node->now() - _lastCmd).seconds();
    if(_lastCmd.nanoseconds() > 0 && age >= 0.0 && age < 0.5){
        setHighCmd(_vx, _vy, _wz);
    }else{
        setHighCmd(0.0, 0.0, 0.0);
    }
}

void State_move_base::twistCallback(const geometry_msgs::msg::Twist &msg){
    if(!std::isfinite(msg.linear.x) || !std::isfinite(msg.linear.y) || !std::isfinite(msg.angular.z)){
        return;
    }
    _vx = msg.linear.x;
    _vy = msg.linear.y;
    _wz = msg.angular.z;
    _lastCmd = _ctrlComp->node->now();
}

#endif  // COMPILE_WITH_MOVE_BASE