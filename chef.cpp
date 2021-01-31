#include "ir.hpp"
#include "servo.hpp"
#include "flux_cam.hpp"
#include "motor_cc.hpp"
#include <pthread.h>
#include <semaphore.h>

#define IR 1

#define SERVO1 22 // 6
#define SERVO2 0 // 17

static pthread_cond_t c = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
sem_t semaphore;
char order = 'l';
double angle_target;
double angle, offset, compteur;


void* start_motor(void *arg)
{
    char init;
    for(int i = 0; i < 2; i++)
        sem_wait(&semaphore);
    
    while ((order != 's') || (abs(angle) < 200))
    {
        do
        {
            printf("Orientez l'encodeur à sa postion initiale. Are you ready? (o or n)\n");
            scanf("%c", &init);
            if (init == 'o')
                offset = compteur;
        } while (init != 'o');

        printf("What do you want to do?\n l -> Left\n r -> Right\n b -> Break\n s -> Stop!\n");
        sleep(1);
        scanf("%c", &order);

        // Lance la commande
        scan(order);
    }
    scan('s');
    return 0;
}


void* IR_listening(void *arg)
{
    int fd = wiringPiI2CSetup (0x04);
    int channel = 1;
    double voltage;

    sem_post(&semaphore);

    while(1)
    {
        voltage = wiringPiI2CReadReg16 (fd, 0x20 + channel) / 1000.0;
        *(double*) arg =  27.86 * pow(voltage, -1.15);
        //printf("Distance = %lf\n", distance);
    }
    return 0;
}

void find_target()
{
    int angle_cam = 180;
    int servo = SERVO2;
    
    // Lancement de la caméra vers l'obstacle détecté le + proche
    movie();
    action(angle_cam, servo);
}

void* encodeur(void *arg)
{
    int fd;
    char buf[20];
    double resolution = (double)360/420;
    
    printf("\nOpening Driver\n");
    fd = open("/dev/encodeur", O_RDONLY);
    if(fd < 0) {
            printf("Cannot open device file...\n");
            return 0;
    }

    read(fd, buf, 20);
    sscanf(buf, "%lf", &compteur);
    offset = compteur;
    angle = (compteur - offset) * resolution;

    sem_post(&semaphore);
    
    while(1) // Regarder si l'angle est inf. à 180°
    {
        read(fd, buf, 20);
        sscanf(buf, "%lf", &compteur);
        angle = (compteur - offset) * resolution;
        //printf("angle = %lf\n compteur = %lf\n", angle, compteur);
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
    pthread_create(&id[0], NULL, IR_listening, &dist);
    pthread_create(&id[1], NULL, encodeur, NULL);
    pthread_create(&id[2], NULL, start_motor, NULL);

    pthread_join(id[0], NULL);
    pthread_join(id[1], NULL);
    pthread_join(id[2], NULL);
    pthread_mutex_unlock(&m);
    sem_destroy(&semaphore);
    

    return 0;
}
