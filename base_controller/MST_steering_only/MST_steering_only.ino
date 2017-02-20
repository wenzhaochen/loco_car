/*
 * rosserial Servo Control Example
 *
 * This sketch demonstrates the control of hobby R/C servos
 * using ROS and the arduiono
 * 
 * For the full tutorial write up, visit
 * www.ros.org/wiki/rosserial_arduino_demos
 *
 * For more information on the Arduino Servo Library
 * Checkout :
 * http://www.arduino.cc/en/Reference/Servo
 */

#if (ARDUINO >= 100)
 #include <Arduino.h>
#else
 #include <WProgram.h>
#endif

#include <Servo.h> 
#include <ros.h>
#include <std_msgs/UInt16.h>
#include <std_msgs/Float64.h>
#include <std_msgs/String.h>
//#include <geometry_msgs/Twist.h>
//#include <ackermann_msgs/AckermannDriveStamped.h>

#define led_pin 13
#define kill_pin 20
#define disable_pin 21
#define esc_pin 22
#define servo_pin 23

ros::NodeHandle  nh;

Servo servo;
Servo esc;

double x;
double w = 0.22;
long steer_zero = 1385;
long steer, throttle;
char buf[200];
unsigned long last_received;
const unsigned long timeout = 1000; //timeout in ms before resetting steering and throttle to 0

bool disabled = 0;
bool kill = 0;


double mapf(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
} 


//void cmd_vel_cb(const geometry_msgs::Twist& cmd_msg){
//  x = cmd_msg.linear.x;
//  w = cmd_msg.angular.z;
//  last_received = millis();
// 
//}

void cmd_vel_cb(const std_msgs::Float64& steering_angle){
//  x = cmd_msg.linear.x;
//  w = cmd_msg.angular.z;
  
  w = steering_angle.data;
  servo.attach(servo_pin,885,1885);
  last_received = millis();
 
}

//ros::Subscriber<geometry_msgs::Twist> sub("cmd_vel", cmd_vel_cb);
ros::Subscriber<std_msgs::Float64> sub("/commands/servo/position", cmd_vel_cb);

std_msgs::String out_msg;
ros::Publisher teensy("teensy", &out_msg);

void setup(){
  pinMode(led_pin, OUTPUT);
  pinMode(esc_pin, OUTPUT);
  pinMode(servo_pin, OUTPUT);
  pinMode(disable_pin, INPUT); 
//  pinMode(kill_pin, INPUT);
//  attachInterrupt(disable_pin, disable_ISR, CHANGE);
//  attachInterrupt(kill_pin, kill_ISR, CHANGE);
  nh.getHardware()->setBaud(115200);
  nh.initNode();
  nh.subscribe(sub);
  nh.advertise(teensy);
  
  servo.attach(servo_pin,885,1885); //attach it to pin A9/23
  esc.attach(esc_pin,1000,2000); //attach it to pin A8/22

  // just to show it's alive
  digitalWrite(led_pin, HIGH);
  delay(100);
  digitalWrite(led_pin, LOW);
  delay(100);
  digitalWrite(led_pin, HIGH);
  delay(100);
  digitalWrite(led_pin, LOW);
}

void loop(){

  unsigned long elapsed = millis() - last_received;
  
//  if (elapsed > timeout) {
////  x = 0;
////    w = 0;
//    
//  }
  if (elapsed > 500 && steer == steer_zero){
      servo.detach();
  }
  
  
  nh.spinOnce();
  String out;
//  out +=  "Throttle: " + String(x) + ", " + String(throttle) + '\t' + "Steering: " + String(w) + ", " + String(steer) + '\t' + "Disabled: " + String(disabled) + "\t Elapsed: " + elapsed ;
out +=  "Steering: " + String(w) + ", " + String(steer) + '\t' + "Disabled: " + String(disabled) + "\t Elapsed: " + elapsed ;
  out.toCharArray(buf,200);
  out_msg.data = buf;
  teensy.publish( &out_msg );

  if (!disabled) {
    
    steer = mapf(w, 0.9977, -0.5577, 885,1885); //maxes out at +/- 0.96 rads = +/- 55 degs
//    servo.attach(servo_pin,885,1885);
    
//    servo.writeMicroseconds(steer); 
//    if (abs(steer-1500) <= 10) {
//          steer = 1500;
//    }

//    esc.writeMicroseconds(throttle);  
    servo.writeMicroseconds(steer);


    
    digitalWrite(led_pin, LOW);
  }
  
  else {  //when disabled
      
    throttle = 1500;
    steer = steer_zero;
    servo.writeMicroseconds(steer);
    servo.detach();
//    esc.writeMicroseconds(1500);
    digitalWrite(led_pin, HIGH);
    
  }

  delay(10);
}

//void disable_ISR() {
//
//  disabled = digitalRead(disable_pin);
////  w = 0;
////  x = 0;
//  throttle = 1500;
//  steer = steer_zero;
//  esc.writeMicroseconds(throttle);
//  servo.writeMicroseconds(steer);
//  servo.detach();
//
//}

