#include <Wire.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// MPU6050 variables
const int MPU_ADDR = 0x68;
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;

// Motor pins (ESP32 PWM)
const int motor1 = 25;
const int motor2 = 26;
const int motor3 = 27;
const int motor4 = 14;

// PWM settings
const int freq = 5000;
const int resolution = 8;

// Channels
const int ch1 = 0;
const int ch2 = 1;
const int ch3 = 2;
const int ch4 = 3;

// Throttle
int throttle = 0;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Drone_Control");

  Wire.begin();
  
  // Wake MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  // Setup PWM
  ledcSetup(ch1, freq, resolution);
  ledcSetup(ch2, freq, resolution);
  ledcSetup(ch3, freq, resolution);
  ledcSetup(ch4, freq, resolution);

  ledcAttachPin(motor1, ch1);
  ledcAttachPin(motor2, ch2);
  ledcAttachPin(motor3, ch3);
  ledcAttachPin(motor4, ch4);
}

void loop() {

  // Read MPU6050
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();

  // Basic tilt estimation
  float angleX = atan(AcY / sqrt(AcX * AcX + AcZ * AcZ)) * 180 / PI;
  float angleY = atan(-AcX / sqrt(AcY * AcY + AcZ * AcZ)) * 180 / PI;

  // Bluetooth throttle input
  if (SerialBT.available()) {
    throttle = SerialBT.parseInt();  // Send values 0–255
  }

  // Constrain throttle
  throttle = constrain(throttle, 0, 255);

  // Basic motor control (no stabilization)
  int m1 = throttle;
  int m2 = throttle;
  int m3 = throttle;
  int m4 = throttle;

  // Write PWM to motors
  ledcWrite(ch1, m1);
  ledcWrite(ch2, m2);
  ledcWrite(ch3, m3);
  ledcWrite(ch4, m4);

  // Debug
  Serial.print("Throttle: "); Serial.print(throttle);
  Serial.print(" | AngleX: "); Serial.print(angleX);
  Serial.print(" | AngleY: "); Serial.println(angleY);

  delay(20);
}
