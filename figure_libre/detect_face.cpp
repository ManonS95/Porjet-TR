#include "detect_face.hpp"
#include <unistd.h>




int main( int argc, const char** argv ) 
{
  envoi_servo_bas(0);
  envoi_servo_haut(0);
  sleep(2);
  go_detect(90);
  return 0; 
} 
