/**********************************************************************
 Copyright (c) 2020-2023, Unitree Robotics.Co.Ltd. All rights reserved.
***********************************************************************/
#include "interface/KeyBoard.h"
#include <algorithm>
#include <poll.h>
#include <unistd.h>

KeyBoard::KeyBoard(){
    userCmd = UserCommand::NONE;
    userValue.setZero();
    if(tcgetattr(STDIN_FILENO, &_oldSettings) != 0){
        return;
    }
    auto settings = _oldSettings;
    settings.c_lflag &= ~(ICANON | ECHO);
    _terminalReady = tcsetattr(STDIN_FILENO, TCSANOW, &settings) == 0;
}

KeyBoard::~KeyBoard(){
    if(_terminalReady){
        tcsetattr(STDIN_FILENO, TCSANOW, &_oldSettings);
    }
}

UserCommand KeyBoard::checkCmd(){
    switch (_c){
    case '1':
        return UserCommand::L2_B;
    case '2':
        return UserCommand::L2_A;
    case '3':
        return UserCommand::L2_X;
    case '4':
        return UserCommand::START;
#ifdef COMPILE_WITH_MOVE_BASE
    case '5':
        return UserCommand::L2_Y;
#endif  // COMPILE_WITH_MOVE_BASE
    case '0':
        return UserCommand::L1_X;
    case '9':
        return UserCommand::L1_A;
    case '8':
        return UserCommand::L1_Y;
    case ' ':
        userValue.setZero();
        return UserCommand::NONE;
    default:
        return UserCommand::NONE;
    }
}

void KeyBoard::changeValue(){
    switch (_c){
    case 'w':case 'W':
        userValue.ly = std::min(userValue.ly+sensitivityLeft, 1.0f);
        break;
    case 's':case 'S':
        userValue.ly = std::max(userValue.ly-sensitivityLeft, -1.0f);
        break;
    case 'd':case 'D':
        userValue.lx = std::min(userValue.lx+sensitivityLeft, 1.0f);
        break;
    case 'a':case 'A':
        userValue.lx = std::max(userValue.lx-sensitivityLeft, -1.0f);
        break;

    case 'i':case 'I':
        userValue.ry = std::min(userValue.ry+sensitivityRight, 1.0f);
        break;
    case 'k':case 'K':
        userValue.ry = std::max(userValue.ry-sensitivityRight, -1.0f);
        break;
    case 'l':case 'L':
        userValue.rx = std::min(userValue.rx+sensitivityRight, 1.0f);
        break;
    case 'j':case 'J':
        userValue.rx = std::max(userValue.rx-sensitivityRight, -1.0f);
        break;
    default:
        break;
    }
}

void KeyBoard::update(){
    pollfd input{STDIN_FILENO, POLLIN, 0};
    if(!_terminalReady || poll(&input, 1, 0) <= 0 || !(input.revents & POLLIN)
            || read(STDIN_FILENO, &_c, 1) != 1){
        return;
    }
    userCmd = checkCmd();
    if(userCmd == UserCommand::NONE){
        changeValue();
    }
}
