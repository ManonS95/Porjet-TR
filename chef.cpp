#include "servo.hpp"
#include "flux_cam.hpp"
#include "motor_cc.hpp"
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <fcntl.h>


#define SERVO1 22 // 6
#define SERVO2 0 // 17

static pthread_cond_t c = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m_stop = PTHREAD_MUTEX_INITIALIZER;
sem_t semaphore;
double angle_target = 0, min_dist = 0;
double angle, offset, compteur;
char stop;

void viderBuffer()
{
    int c = 0;
    while (c != '\n' && c != EOF)
    {
        c = getchar();
    }
}

int kbhit()
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}

void* detect_stop(void* arg)
{
    pthread_mutex_lock(&m_stop);
    printf("Press 's' to stop the lidar!\n");
    do
    {
        if( kbhit() )
        {
            stop = getchar();
		}
        
    } while(stop != 's');
    return 0;
}

void* start_motor(void *arg)
{
    char init;
    bool direction = true;
    for(int i = 0; i < 2; i++)
        sem_wait(&semaphore);
    
    do
    {
        printf("Orientez l'encodeur à sa position initiale. Are you ready? (o or n)\n");
        scanf("%c", &init);
        
        if (init == 'o')
        {
            sem_wait(&semaphore);
            offset = compteur;
        }
    } while (init != 'o');

    pthread_mutex_unlock(&m_stop);

    do
    {
        //printf("angle_target = %lf\n angle = %lf\n offset = %lf\n compteur = %lf\n", angle_target, angle, offset, compteur);
        // printf("angle_target = %lf\n", angle_target);
        // Lance la commande
        if (direction == true)
        {
            offset = compteur;
            scan('r');
        }
        else
            scan('l');
        
        // Attendre que le moteur ait fini son tour
        if (angle < 20 && angle > 170)
        {
            pthread_mutex_unlock(&m);
            sleep(1);
            pthread_mutex_lock(&m);
        }
        direction = !direction;
        min_dist = 0;
        sleep(3);
        
    } while(stop != 's');
    
    scan('s');
    return 0;
}


void* IR_listening(void *arg)
{
    int fd = wiringPiI2CSetup(0x04);
    int channel = 1;
    double voltage, distance;

    sem_post(&semaphore);

    while(1)
    {
        voltage = wiringPiI2CReadReg16(fd, 0x20 + channel) / 1000.0;
        distance =  27.86 * pow(voltage, -1.15);
        //printf("distance = %lf\n min_dist = %lf\n", distance, min_dist);
        if ((min_dist > distance) || (min_dist == 0))
        {
            min_dist = distance;
            angle_target = angle;
        }
    }
    return 0;
}


void* find_target(void* arg)
{
    int servo = SERVO2;
    
    // Lancement de la caméra vers l'obstacle détecté le + proche
    //movie();
    while(1)
    {
        if (angle_target > 20 && angle_target < 170)
        {
            
            printf("angle_target = %lf\n", angle_target);
            action(angle_target, servo);
            
        }
    }
    
}


void* encodeur(void *arg)
{
    int fd;
    char buf[20];
    double resolution = (double)90/47;//(double)360/420;
    double angle_prec;
    clock_t start, now;
    int msec = 0;
    
    printf("\nOpening Driver\n");
    fd = open("/dev/encodeur", O_RDONLY);
    if(fd < 0) 
    {
        printf("Cannot open device file...\n");
        return 0;
    }

    read(fd, buf, 20);
    sscanf(buf, "%lf", &compteur);
    offset = compteur;
    angle_prec = (compteur - offset) * resolution;
    angle_prec = angle;
    start = clock();
    
    sem_post(&semaphore);

    while(1) // Regarder si l'angle est inf. à 180°
    {
        read(fd, buf, 20);
        sscanf(buf, "%lf", &compteur);
        angle = (compteur - offset) * resolution;
        //printf("angle = %lf\n compteur = %lf\n", angle, compteur);
        //printf("compteur - offset = %lf\n", compteur - offset);
        
        clock_t diff = clock() - start;
        msec = diff * 1000 / CLOCKS_PER_SEC;
        //printf("msec = %d\n", msec);
        if (angle_prec != angle)
        {
            //printf("Start = now\n");
            start = clock(); // On réinitialise le timer
        }
        if (msec > 10) // Si ça fait plus de 1 seconde que le moteur ne bouge plus
        {
            //printf("> 3\n");
            // Libère le mutex
            pthread_mutex_unlock(&m);
            sleep(1);
        }
        angle_prec = angle;
        sem_post(&semaphore);
    }

    close(fd);
    return 0;
}


int main()
{
    double dist = 0;

    // On lance le moteur CC
    // On check les capteurs
    // Si tour terminé -> lancement caméra

    pthread_t id[5];
    sem_init(&semaphore, 0, 0);
    pthread_mutex_lock(&m);
    pthread_mutex_lock(&m_stop);
    pthread_create(&id[0], NULL, IR_listening, NULL);
    pthread_create(&id[1], NULL, encodeur, NULL);
    pthread_create(&id[2], NULL, start_motor, NULL);
    pthread_create(&id[3], NULL, find_target, NULL);
    pthread_create(&id[4], NULL, detect_stop, NULL);


    pthread_join(id[0], NULL);
    pthread_join(id[1], NULL);
    pthread_join(id[2], NULL);
    pthread_join(id[3], NULL);
    pthread_join(id[4], NULL);
    sem_destroy(&semaphore);
    

    return 0;
}
