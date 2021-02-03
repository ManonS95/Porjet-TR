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


#define SERVO1 0 // 17
#define SERVO2 23 // 13

static pthread_cond_t c = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m_stop = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m_offset = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m_target= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m_angle= PTHREAD_MUTEX_INITIALIZER;
sem_t semaphore;
double angle_target = 0, min_dist = 0;
double angle, offset, compteur;
char stop;


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
    for(int i = 0; i < 3; i++)
        sem_wait(&semaphore);
   
    do
    {
        printf("Orientez l'encodeur à sa position initiale. Are you ready? (o or n)\n");
        scanf("%c", &init);
       
        if (init == 'o')
        {
            pthread_mutex_lock(&m_offset);
            offset = compteur;
            pthread_mutex_unlock(&m_offset);
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
            scan('r');
            while(angle < 20) {printf("angle = %lf \n", angle);}
        }
        else
        {
            scan('l');
            while(angle > -20) {printf("angle = %lf \n", angle);}
        }
       
        scan('s');
        direction = !direction;
        pthread_mutex_lock(&m_target);
        if (min_dist == 0)
            printf("Pas d'obstacle détecté ! \n");
            
        printf("angle target = %lf \n", angle_target);
        action(angle_target, SERVO2);
        min_dist = 0;
        pthread_mutex_unlock(&m_target);
       
        sleep(1);
       
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
        pthread_mutex_lock(&m_target);
        if ((min_dist > distance) || (min_dist == 0))
        {
            min_dist = distance;
            angle_target = angle + 90;
        }
        pthread_mutex_unlock(&m_target);
    }
    return 0;
}



void* find_target(void* arg)
{   
    // Lancement de la caméra vers l'obstacle détecté le + proche
    VideoCapture camera;
    // open the first webcam plugged in the computer
    if (!camera.open(0)) {
        std::cerr << "ERROR: Could not open camera" << std::endl;
    }
    
    sem_post(&semaphore);
    movie(camera);
    
    return 0;
}


void* encodeur(void *arg)
{
    int fd;
    char buf[20];
    double resolution = 1.2 ;//(double)360/420;
   
   
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
    pthread_mutex_unlock(&m_angle);
    angle = (compteur - offset) * resolution;
    pthread_mutex_unlock(&m_angle);
   
    sem_post(&semaphore);

    while(1) // Regarder si l'angle est inf. à 180°
    {
        read(fd, buf, 20);
        pthread_mutex_lock(&m_offset);
        sscanf(buf, "%lf", &compteur);
        angle = (compteur - offset) * resolution;
        pthread_mutex_unlock(&m_offset);
        //printf("angle = %lf\n compteur = %lf\n", angle, compteur);
        //printf("compteur - offset = %lf\n", compteur - offset);
   
    }

    close(fd);
    return 0;
}


int main()
{
    pthread_t id[5];
    sem_init(&semaphore, 0, 0);
    pthread_mutex_lock(&m);
    pthread_mutex_lock(&m_stop);
    scan('s');
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
