all: face_reco

face_reco: detect_face.o
	g++ -Wall -o detect_face detect_face.cpp -I /usr/local/include -L/usr/local/lib -lm -lopencv_highgui -lopencv_imgproc -lopencv_core -lopencv_imgcodecs -lopencv_objdetect -lopencv_videoio -lwiringPi

detect_face.o: detect_face.cpp detect_face.hpp servo.h
	g++ -Wall -I /usr/local/include -L/usr/local/lib -c detect_face.cpp -lm -lopencv_highgui -lopencv_imgproc -lopencv_core -lopencv_imgcodecs -lopencv_objdetect -lopencv_videoio -lwiringPi

clean:
	rm -f *- *.o

mrproper: clean
	rm -f face_reco
