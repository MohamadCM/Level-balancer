#include <Wire.h>  // Wire library - used for I2C communication
#include <Servo.h>  // Servo library
#include <Arduino_FreeRTOS.h> // Real time OS

 
Servo servo;  
int servoPin = 9;
int servoAngle = 80;   // servo's starting position in degrees

int ADXL345 = 0x53; // The ADXL345 sensor I2C address

int x,y,z;
float xg,yg,zg;
float soh;
float tilt;
float angle;
String dir;
int highPriority = 1;
int lowPriority = 0;
void calculateTilt();
void rotateServo();

void setup(){
  Serial.begin(9600);
  Serial.print("Angle Meter");  
  Serial.begin(9600);
  servo.attach(servoPin);
  Wire.begin(); // Initiate the Wire library
  
  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345); // Start communicating with the device 
  Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(0x8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable 
  Wire.endTransmission();
  servo.write(servoAngle);
  
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
                    
    //Accelerometer stuff   
    Wire.beginTransmission(ADXL345);
    Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(ADXL345, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
    x = ( Wire.read()| Wire.read() << 8); // X-axis value
    y = ( Wire.read()| Wire.read() << 8); // Y-axis value
    z = ( Wire.read()| Wire.read() << 8); // Z-axis value
    
    xg = x*0.0039;
    yg = y*0.0039;
    zg = z*0.0039;
    soh = yg/zg;
  
    tilt = (int)(atan(soh)*57.0);
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
     servo.write(servoAngle + tilt);
     
     vTaskPrioritySet(NULL, //Curent task
                    lowPriority); // Task can be interrupted
  }
}
