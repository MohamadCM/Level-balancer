#include <Wire.h>
#include <Wire.h>  // Wire library - used for I2C communication
#include <Servo.h>  //add '<' and '>' before and after servo.h
 
int servoPin = 9;
 
Servo servo;  
 
int servoAngle = 80;   // servo position in degrees

int ADXL345 = 0x53; // The ADXL345 sensor I2C address
int x,y,z;
float xg,yg,zg;
float soh;
float tilt;
float angle;
String dir;

uint8_t degree[8]  = {140,146,146,140,128,128,128,128};

void setup(){
  Serial.begin(9600);
  Serial.print("Angle Meter");  
  Serial.begin(9600);
  servo.attach(servoPin);
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
 // adxl.printAllRegister();
  Wire.begin(); // Initiate the Wire library
  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345); // Start communicating with the device 
  Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable 
  Wire.endTransmission();
  servo.write(servoAngle);
  delay(1000);
}

void loop(){
  
  //Boring accelerometer stuff   
 // adxl.readAccel(&x, &y, &z); //read the accelerometer values and store them in variables  x,y,z
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
  // Output x,y,z values - Commented out
//Serial.print("X Value: "); Serial.print(xg);
//Serial.print(", Y Value: "); Serial.print(yg);
//Serial.print(", Z Value: "); Serial.println(zg);
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
    servo.write(servoAngle + tilt);
    
    Serial.print("Tilt:");Serial.print(angle);Serial.write(byte(0));Serial.print(dir);
    Serial.println("");
  }
//Serial.print("Tilt Angle is: "); Serial.print(tilt); Serial.println(" degrees.");


 delay(1000);
}
