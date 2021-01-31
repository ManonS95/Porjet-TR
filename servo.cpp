#include "servo.hpp"

void action(int angle, int servo)
{
    int left, i, pulse;

    if (wiringPiSetup () == -1)
    {
        fprintf (stdout, "oops: %s\n", strerror (errno)) ;
    }
    
    if (angle > 180 || angle < 0)
    {
        angle = 0;
    }
    printf("angle to send : %d\n",angle);
    pinMode(servo, OUTPUT);
    digitalWrite(servo, 0);

    // calcul de pulse
    pulse = (angle*(1820/180)) + 600;
    printf("pulse to send : %d\n", pulse);
    left = 20000 - pulse;
    i = 0;

    while ( i < 35 )
    {
        i++;
        digitalWrite(servo, 1);
        delayMicroseconds(pulse);
        digitalWrite(servo, 0);
        delayMicroseconds(left);
    }
}