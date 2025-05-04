#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>

/* Assign a unique ID to the sensors */
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);

void setup(void) {
  Serial.begin(115200);
  Serial.println("LSM303 Accelerometer + Magnetometer Test");

  /* Initialize the sensors */
  if (!accel.begin() || !mag.begin()) {
    if (!accel.begin()) {
      Serial.println("Failed to initialize the accel(s)");
    }
    if (!mag.begin()) {
      Serial.println("Failed to initialize the mag(s)");
    }
    while (1)
      ;
  }
}

void loop(void) {
  /* Get a new sensor event */
  sensors_event_t event;

  /* Read the accelerometer */
  accel.getEvent(&event);
  Serial.print("Accel_X:");
  Serial.print(event.acceleration.x);
  Serial.print(",");
  Serial.print("Accel_Y:");
  Serial.print(event.acceleration.y);
  Serial.print(",");
  Serial.print("Accel_Z:");
  Serial.print(event.acceleration.z);
  // Serial.print(" m/s^2");

  /* Read the magnetometer */
  mag.getEvent(&event);
  Serial.print(",Mag_X:");
  Serial.print(event.magnetic.x);
  Serial.print(",");
  Serial.print("Mag_Y:");
  Serial.print(event.magnetic.y);
  Serial.print(",");
  Serial.print("Mag_Z:");
  Serial.println(event.magnetic.z);
  // Serial.println(" uT");

  /* Delay before the next reading */
  delay(100);
}