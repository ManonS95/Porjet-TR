#include "motor_cc.hpp"
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdio.h>


void scan(char choice)
{

    int fd;

    fd = wiringPiI2CSetup(GROVE_MOTOR_DRIVER_DEFAULT_I2C_ADDR);
    int speed = 100; // Entre 0 et 255
    int pin_ = 0;
    int cmd = (speed << 8) | (pin_ & 0xFF);


    switch(choice){
    case 'r':
        wiringPiI2CWriteReg16(fd, CMD_CW, cmd);
        break;
    case 'l':
        wiringPiI2CWriteReg16(fd, CMD_CCW, cmd);
        break;
    case 'b':
        wiringPiI2CWriteReg16(fd, CMD_BRAKE, 0x00);
        break;
    case 's':
        wiringPiI2CWriteReg16(fd, CMD_STOP, 0x00);
        break;
    }

}
