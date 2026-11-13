/**********************************************************************
 Copyright (c) 2020-2023, Unitree Robotics.Co.Ltd. All rights reserved.
***********************************************************************/
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <termios.h>
#include "interface/CmdPanel.h"

class KeyBoard : public CmdPanel{
public:
    KeyBoard();
    ~KeyBoard() override;
    void update();

private:
    UserCommand checkCmd();
    void changeValue();

    float sensitivityLeft = 0.05;
    float sensitivityRight = 0.05;
    struct termios _oldSettings{};
    bool _terminalReady = false;
    char _c = '\0';
};

#endif  // KEYBOARD_H
