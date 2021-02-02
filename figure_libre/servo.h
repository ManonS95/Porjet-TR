#ifndef SERVO_H
#define SERVO_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <wiringPi.h>
#define SERVO 23 // pin 13 pwm
#define SERVO_HAUT 0 //pin 16 pwm
//pour connaitre la pin utiliser la table de conversion BCM Wiring PI et mettre dans define le numero wiring pi


void envoi_servo_bas(int angle){
  
    int left, i, pulse;
    if (wiringPiSetup () == -1)
    {
      fprintf (stdout, "oops: %s\n", strerror (errno)) ;

    }
    if (angle > 180 || angle < 0 )
      {
	angle = 0;
	printf("angle demandé %d  non compris entre 0° et 180° \n On envoie le servomoteur à 0° par défault \n", angle);
      }
    //printf("par ici \n");
    
    printf("angle to send : %d\n",angle);
    
    pinMode(SERVO,OUTPUT);
    digitalWrite(SERVO,0);
    // calcul de pulse en microsecondes
    
    int pulse_droite= 570; //butée droite correspondant à 0°
    int pulse_gauche= 2500; //butée gauche correspondant à 180°
    pulse=pulse_droite+ (pulse_gauche-pulse_droite)*angle/180; //interpolation
    printf("pulse to send : %d\n",pulse);
    left=20000-pulse;
    i=0;
    
    while ( i < 35 )
      {
        i++;
        digitalWrite(SERVO,1);
        delayMicroseconds(pulse);
        digitalWrite(SERVO,0);
        delayMicroseconds(left);
      }
     
}

void envoi_servo_haut(int angle){
  
    int left, i, pulse;
    if (wiringPiSetup () == -1)
    {
      fprintf (stdout, "oops: %s\n", strerror (errno)) ;

    }
    if (angle > 180 || angle < 0 )
      {
	angle = 0;
	printf("angle demandé %d  non compris entre 0° et 180° \n On envoie le servomoteur à 0° par défault \n", angle);
      }
    //printf("par ici \n");
    
    printf("angle to send : %d\n",angle);
    
    pinMode(SERVO_HAUT,OUTPUT);
    digitalWrite(SERVO_HAUT,0);
    // calcul de pulse en microsecondes
    
    int pulse_droite= 570; //butée droite correspondant à 0°
    int pulse_gauche= 2500; //butée gauche correspondant à 180°
    pulse=pulse_droite+ (pulse_gauche-pulse_droite)*angle/180; //interpolation
    printf("pulse to send : %d\n",pulse);
    left=20000-pulse;
    i=0;
    
    while ( i < 35 )
      {
        i++;
        digitalWrite(SERVO_HAUT,1);
        delayMicroseconds(pulse);
        digitalWrite(SERVO_HAUT,0);
        delayMicroseconds(left);
      }
     
}

#endif
