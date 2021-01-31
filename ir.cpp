#include "ir.hpp"

double mesure()
{
    int fd = wiringPiI2CSetup (0x04);
    int channel = 1;
    double voltage, distance;

    voltage = wiringPiI2CReadReg16 (fd, 0x20 + channel) / 1000.0;
    distance =  27.86 * pow(voltage, -1.15);
    //printf("Distance = %lf\n", distance);

    return distance;
}