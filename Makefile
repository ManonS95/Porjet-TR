GCC:=g++
LIBS:=-lpthread -lwiringPi -lm -I /usr/local/include -L /usr/local/lib -lopencv_core -lopencv_highgui -lopencv_videoio
FLAGS:=-Wall -pthread
all: chef

chef: chef.o servo.o flux_cam.o motor_cc.o
	$(GCC) $(FLAGS) $(LIBS) -o chef $^

clear:
	rm -f *.o

xclear: clear
	rm -f all