#include "flux_cam.hpp"

void movie(VideoCapture camera)
{    
    // Display the frame until you press a key
    while (1) 
    {
        // This will contain the image from the webcam
        Mat frame;
        // capture the next frame from the webcam
        camera >> frame;
        if( frame.empty() )
            break; // end of video stream
        // show the image on the window
        imshow("Webcam", frame);
        // wait (10ms) for a key to be pressed
        if (waitKey(10) == 27)
            break;
    }
}
