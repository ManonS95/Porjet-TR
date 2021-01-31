#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc, char *argv[])
{
        int fd;
        char buf2[20];
        double angle, offset;
        double resolution = (double)360/420;
        long compteur;
        char init[10];
 
        printf("\nOpening Driver\n");
        fd = open("/dev/encodeur", O_RDONLY);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }
        read(fd, buf2, 20);

        do
        {
                printf("Initialisation\n Placez le capteur en butée.\n Le capteur est bien positionné ? (Oui ou Non)\n");
                scanf("%s", init);
        } while (strcmp(init, "Oui") != 0);

        compteur = atoi(buf2);
        offset = compteur;
        angle = (compteur - offset) * resolution;
        printf("angle = %lf\n offset = %lf\n", angle, offset);
 
        printf("Closing Driver\n");
        close(fd);
        return 0;
}