#ifndef DETECT_FACE_HPP
#define DETECT_FACE_HPP

#include "opencv2/objdetect.hpp" 
#include "opencv2/highgui.hpp" 
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "servo.hpp"

#include <iostream>
#include <cstdio>

using namespace std; 
using namespace cv;

#define SERVO_HAUT 22//0 // 17
#define SERVO_BAS 0//23 // 13

// Function for Face Detection

void detectAndDraw( Mat& img, CascadeClassifier& cascade, CascadeClassifier& nestedCascade, double scale);

void go_detect(int angle);


		

#endif
