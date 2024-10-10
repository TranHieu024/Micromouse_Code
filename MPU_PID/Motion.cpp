
#include "Motion.h"

void Motion::set_pin(){

  // set left mt left: 
  pinMode(32,OUTPUT);
  pinMode(33,OUTPUT);

  // set pin mt right: 
  pinMode(25,OUTPUT);
  pinMode(26,OUTPUT);

  // set standby pin
  pinMode(5,OUTPUT);
  
}


void Motion::stop(){
  analogWrite(32,20);
  analogWrite(33,0);  
  analogWrite(25,20);
  analogWrite(26,0);

  
}
void Motion:: turn_left(){
  
  analogWrite(32,255);
  analogWrite(33,0);  
  analogWrite(25,0);
  analogWrite(26,255);
}
void Motion::set_target(float first_read_from_imu){
  target = first_read_from_imu;
}

float Motion::get_target(){
  return target;
}

void Motion::speed_increase(){
  
    if ( speed_tb <= speed_target)
      speed_tb+=5 ;
}

void Motion::set_error(float now){
//  error = (now - target) * 100  ; // imu

  error = now - target; // tof
}

float Motion::get_error(){
  return error;
}

void Motion::set_Pvalue(int x){

  if (x)
    Pvalue += 0.1;
  else 
    Pvalue -= 0.1;

//// set speed for left wheel
//  if ( x)
//  left+= 5;
//  else 
//  left -= 5;
}
void Motion::set_speed(int lsp, int rsp){
  speed_increase();
  
  if ( rsp < 0 ){
    analogWrite(25,-rsp);
    analogWrite(26,0);   
  }
  else{
    analogWrite(25,0);
    analogWrite(26,rsp);
  }

  if ( lsp < 0 ){
  analogWrite(32,-lsp);
  analogWrite(33,0);
  }
  else {
  analogWrite(32,0);
  analogWrite(33,lsp);    
  }
}

void Motion::run_straight(){
  analogWrite(32,0);
  analogWrite(33,left);  
  analogWrite(25,0);
  analogWrite(26,70);
}


void Motion::set_state(int state){
  
  if (state)
    digitalWrite(5,HIGH);
 
  else 
    digitalWrite(5,LOW);

}

void Motion::set_speed_tb(int s_tb){
  speed_tb = s_tb;
}
void Motion::run_pid() { 
  int lsp,rsp; 
  P = error;
  I = error + I;
  D = error - previous_error;
  
  float PIDvalue = Pvalue*P  + Dvalue*D + Ivalue*I;

  previous_error = error;
      lsp = speed_tb + PIDvalue;
      rsp = speed_tb - PIDvalue;
//      
//   } else{
//       lsp = speed_tb - PIDvalue ;
//       rsp = speed_tb + PIDvalue;
//   }
//   lsp = speedmax + PIDvalue;
//   rsp = speedmax - PIDvalue;

   if (lsp > speed_max) {
      lsp = speed_max;
    }
   else if (lsp < -speed_max) {
      lsp = -speed_max;
    }
   if (rsp > speed_max) {
      rsp = speed_max;
    }
    else if (rsp < -speed_max) {
      rsp = -speed_max;
    }
  set_speed(lsp,rsp);
}
