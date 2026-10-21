#ifndef ESP32_FC28_H
#define ESP32_FC28_H

#include <Arduino.h>

// Driver for the FC-28 resistive soil moisture sensor, read via the ESP32's
// ADC1. Returns a 0.0-100.0 percentage, linearly scaled from the raw 12-bit
// ADC reading (0-4095). Higher analog reading = drier soil for this sensor
// wired as a simple voltage divider (resistance rises as the soil dries),
// so a HIGHER percentage here means DRIER soil, not wetter. See the comment
// on getSoilMoisture() in FC28.cpp - main.ino's pump logic depends on this
// convention being correct.
class FC28Sensor {
  public:
    // Call once from setup(). Does NOT touch Serial - the application
    // (main.ino) owns Serial.begin(), not the sensor driver.
    void initFC28Sensor(int pin);
    float getSoilMoisture();

  private:
    int _pin;
    float analogFC28, value;
};

#endif