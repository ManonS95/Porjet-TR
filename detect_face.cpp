#include "detect_face.hpp"


void detectAndDraw(Mat& img, CascadeClassifier& cascade, CascadeClassifier& nestedCascade, double scale) 
{
    bool detected = false;
  
    vector<Rect> faces, faces2; 
    Mat gray, smallImg; 

    cvtColor( img, gray, COLOR_BGR2GRAY ); // Convert to Gray Scale 
    double fx = 1 / scale; 

    // Resize the Grayscale Image 
    resize( gray, smallImg, Size(), fx, fx, INTER_LINEAR ); 
    equalizeHist( smallImg, smallImg ); 

    // Detect faces of different sizes using cascade classifier 
    cascade.detectMultiScale( smallImg, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) ); 

    // Draw circles around the faces 
    for ( size_t i = 0; i < faces.size(); i++ ) 
    {
	detected=true;
	Rect r = faces[i]; 
	Mat smallImgROI; 
	vector<Rect> nestedObjects; 
	Point center; 
	Scalar color = Scalar(0, 0, 255); // Color for Drawing tool in BGR 
	int radius;
	double aspect_ratio = (double)r.width/r.height; 
	if( 0.75 < aspect_ratio && aspect_ratio < 1.3 ) 
	{ 
	    center.x = cvRound((r.x + r.width*0.5)*scale); 
	    center.y = cvRound((r.y + r.height*0.5)*scale); 
	    radius = cvRound((r.width + r.height)*0.25*scale); 
	    circle( img, center, radius, color, 3, 8, 0 ); 
	} 
	else
	    rectangle( img, cvPoint(cvRound(r.x*scale), cvRound(r.y*scale)), 
			    cvPoint(cvRound((r.x + r.width-1)*scale), 
			    cvRound((r.y + r.height-1)*scale)), color, 3, 8, 0); 
	if( nestedCascade.empty() ) 
	    continue; 
	smallImgROI = smallImg( r );
	// Detection of eyes int the input image 
	nestedCascade.detectMultiScale( smallImgROI, nestedObjects, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) ); 
	
	// Draw circles around eyes 
	for ( size_t j = 0; j < nestedObjects.size(); j++ ) 
	{ 
	    Rect nr = nestedObjects[j]; 
	    center.x = cvRound((r.x + nr.x + nr.width*0.5)*scale); 
	    center.y = cvRound((r.y + nr.y + nr.height*0.5)*scale); 
	    radius = cvRound((nr.width + nr.height)*0.25*scale); 
	    circle( img, center, radius, color, 3, 8, 0 ); 
	} 
    }
    
    if (detected == true)
    { //add warning text
	putText(img, "Warning : Human Face Detected! ", Point(15,20), FONT_HERSHEY_COMPLEX_SMALL, 1.0, Scalar(0,0,255), 2, CV_AA);
	putText(img, "Deployement of the security service", Point(15,100), FONT_HERSHEY_COMPLEX_SMALL, 1.0, Scalar(0, 0, 255), 2, CV_AA);
	putText(img, "in progress...", Point(15,400), FONT_HERSHEY_COMPLEX_SMALL, 1.0, Scalar(0,0,255), 2, CV_AA);
    }
    else
    { //add safety text
	putText(img, "No Human Face Detected", Point(15,30), FONT_HERSHEY_COMPLEX_SMALL, 2.0, Scalar(0,255,0), 2, CV_AA);
    }

    // Show Processed Image with detected faces 
    imshow( "Face Detection", img ); 
}




void go_detect(int angle)
{
    action(angle, SERVO_BAS);
    action(90, SERVO_HAUT);

    // VideoCapture class for playing video for which faces to be detected 
    VideoCapture capture; 
    Mat frame, image; 

    // PreDefined trained XML classifiers with facial features 
    CascadeClassifier cascade, nestedCascade; 
    double scale = 1; 

    // Load classifiers from "opencv/data/haarcascades" directory
    //pas besoin j'ai mis le fichier dans le dossier
    nestedCascade.load( "haarcascade_eye_tree_eyeglasses.xml" ) ; 

    // Change path before execution
    //pas besoin j'ai mis le fichier dans le dossier
    cascade.load( "haarcascade_frontalcatface.xml" ) ; 

    // Start Video..1) 0 for WebCam 2) "Path to Video" for a Local Video 
    capture.open(0); 
    if( capture.isOpened() ) 
    { 
        // Capture frames from video and detect faces 
        cout << "Face Detection Started...." << endl; 
        while(1) 
        {
            capture >> frame; 
            if( frame.empty() ) 
                break; 
            Mat frame1 = frame.clone(); 
            detectAndDraw( frame1, cascade, nestedCascade, scale );
            char c = (char)waitKey(10); 
            
            // Press q to exit from window 
            if( c == 27 || c == 'q' || c == 'Q' ) 
                break; 
        }
    } 
    else
        cout<<"Could not Open Camera"; 
}
