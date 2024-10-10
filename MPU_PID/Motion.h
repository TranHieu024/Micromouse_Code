#ifndef __Motion.h__
#define __Motion.h__

#include <Arduino.h>


class Motion {
  private :
    int speed_max = 200;  
    int speed_tb = 100; 
    int speed_target = 200;
    int state = 1;
    int P; 
    int D; 
    int I;
    
    int Pvalue = 3; 
    int Dvalue = 2; 
    int Ivalue = 0;
    int previous_error;
    
  public: 
    float target;
    float error ; 
    int left = 70;
    void set_pin();
    void run_pid();
    void set_speed(int lsp, int rsp);
    void set_error(float now);
    void set_target(float first_read_from_imu);
    void set_Pvalue(int x);
    void set_state(int state);
    void set_speed_tb( int s_tb);

        
    float get_error();
    float get_target();
  
    void run_straight();
    void turn_left();
    void speed_increase();
    void stop();

};

#endif
