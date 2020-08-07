#include  <Wire.h>                    // Wire library - used for I2C communication
#include  <Servo.h>                   // Servo library
#include  <Arduino_FreeRTOS.h>        // Real time OS

#define   precision           0.0039 
#define   calibration_factor  57.0
#define   servo_pin           9       // Servo's connection pin
#define   servo_angle         80      // servo's starting position in degrees
#define   ADXL345             0x53    // The ADXL345 sensor I2C address
#define   highPriority        1
#define   lowPriority         0
 
//Global Variables
Servo servo; 
float tilt;

//Tasks
void calculateTilt();
void rotateServo();

void setup(){
  Serial.begin(9600);
  Serial.print("Angle Meter");  
  Serial.begin(9600);
  servo.attach(servo_pin);
  Wire.begin(); // Initiate the Wire (I2C) library
  
  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345); // Start communicating with the device 
  Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(0x8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable 
  Wire.endTransmission();
  servo.write(servo_angle);
  
  xTaskCreate(calculateTilt, 
              "Caculate tilt", 
              150, //Stack depth
              NULL, 
              0, //Priority
              NULL);
  xTaskCreate(rotateServo, 
              "Rotate servo", 
              50, //Stack depth
              NULL, 
              0, //Priority
              NULL);
}

void loop(){}

void calculateTilt(void * unused){
  while(true){
    vTaskPrioritySet(NULL, //Curent task
                    highPriority); // Task won't be interrupted

    //Initial Values
    int     x, y, z;
    float   xg, yg, zg;
    float   soh;
    float   angle;
    String  dir;
    //Accelerometer stuff   
    Wire.beginTransmission(ADXL345);
    Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
    x = ( Wire.read()| Wire.read() << 8); // X-axis value
    y = ( Wire.read()| Wire.read() << 8); // Y-axis value
    z = ( Wire.read()| Wire.read() << 8); // Z-axis value
    
    xg = x * precision;
    yg = y * precision;
    zg = z * precision;
    soh = yg/zg;
    tilt = (int)(atan(soh) * calibration_factor);
    
    if (abs(tilt) > 90) {
        Serial.print("Tilt:Range Error");
      }
      else {
        if (tilt < 0) {
          dir = "Up";
          angle = abs(tilt);
        } else {
          dir = "Down";
          angle = tilt;
        }
    
        Serial.print("Tilt:");
        Serial.print(angle);
        Serial.write(byte(0));
        Serial.println(dir);
      }
    Serial.print("Tilt Angle is: "); 
    Serial.print(tilt); 
    Serial.println(" degrees.");
    vTaskPrioritySet(NULL, //Curent task
                    lowPriority); // Task can be interrupted      
  }          
}
void rotateServo(void * unused){
  while(true){
     vTaskPrioritySet(NULL, //Curent task
                    highPriority); // Task won't be interrupted
                    
     Serial.println("Rotating Servo");
     servo.write(servo_angle + tilt);
     
     vTaskPrioritySet(NULL, //Curent task
                    lowPriority); // Task can be interrupted
  }
}
