#include "FC28.h"

void FC28Sensor::initFC28Sensor(int pin) {
  pinMode(pin, INPUT);
  _pin = pin;
}

// Returns a linear 0.0-100.0 scaling of the raw 12-bit ADC reading (0-4095),
// i.e. value = analogFC28 / 40.95. This reproduces exactly what the original
// 900-line lookup table was trying to do (every correct threshold in that
// table was an exact multiple of 40.95 = 4095/100), without the three
// unreachable buckets caused by transposed comparison bounds in the original
// (e.g. ">= 3788.735 && < 3742.83", which can never be true).
float FC28Sensor::getSoilMoisture() {
  analogFC28 = analogRead(_pin);
  value = analogFC28 / 40.95;

  // Clamp defensively - analogRead() on ESP32 is specified 0-4095, but
  // clamping protects against any future change to attenuation/resolution.
  if (value > 100.0) value = 100.0;
  if (value < 0.0) value = 0.0;

  return value;
}