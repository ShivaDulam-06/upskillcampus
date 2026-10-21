// HEADER FILES ---------------------------
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "DHT.h"
#include <RTClib.h>
RTC_DS1307 rtc;
// ----------------------------------------

// VARIABLE DEFINITIONS -------------------
#define relayPin 18
#define ldrPin 35
#define DHT_PIN 15
#define DHTTYPE DHT22
#define LED_PIN 33
DHT dht(DHT_PIN, DHT22);
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Soil Moisture Sensor (FC-28) with Potentiometer
#include "FC28.h"
#define SensorFC28_Pin 34
float soil_moisture;
FC28Sensor mySensor;

// ESP32 ADC: 12-bit (0-4095), ~3.3V reference. The photoresistor module
// already includes its own conditioning circuit, so this just converts the
// raw reading to volts, then to an approximate lux value using the standard
// LDR resistance/lux relationship.
const float ADC_MAX = 4095.0;
const float ADC_VREF = 3.3;
const float gama = 0.7;
const float rl10 = 50;

String soilCondition;
String lightLevel;
float humidity;
float temperature;
float lux;
// ----------------------------------------

// SETUP FUNCTION -------------------------
void setup() {
  // Serial is initialized exactly once, here, by the application.
  // (FC28Sensor no longer touches Serial internally - see FC28.cpp.)
  Serial.begin(9600);

  pinMode(ldrPin, INPUT);
  pinMode(DHT_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(relayPin, OUTPUT);
  mySensor.initFC28Sensor(SensorFC28_Pin);

  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("RTC Not Connected");
    lcd.setCursor(0, 0);
    lcd.print("RTC Not Connected");
    while (1);
  }

  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to");
  lcd.setCursor(0, 1);
  lcd.print("Smart Agriculture");
  lcd.setCursor(0, 2);
  lcd.print("Monitoring System");
  delay(2000);
  lcd.clear();
  Serial.println("Smart Agriculture System Initialized");
}
// ----------------------------------------

// MAIN LOOP ------------------------------
void loop() {
  DateTime now = rtc.now();
  Serial.println("Date               : " + String() + now.day() + "/" + now.month() + "/" + (now.year() % 100));
  Serial.println("Time               : " + String() + now.hour() + ":" + now.minute() + ":" + now.second());

  // Read sensor values
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  // LDR: ESP32 ADC is 12-bit / ~3.3V, read and converted directly - no
  // intermediate rescale needed.
  int ldrValue = analogRead(ldrPin);
  float ldrVoltage = ldrValue / ADC_MAX * ADC_VREF;
  float ldrResistance = 2000 * ldrVoltage / (1 - ldrVoltage / ADC_VREF);
  lux = pow(rl10 * 1e3 * pow(10, gama) / ldrResistance, (1 / gama));

  // Soil moisture: see the convention note in FC28.h / FC28.cpp - higher
  // percentage here means DRIER soil (voltage-divider behavior of this
  // sensor), which is why controlPump() triggers the pump at HIGH values.
  soil_moisture = mySensor.getSoilMoisture();

  // Interpret sensor values
  soilCondition = soil_moisture < 33.33 ? "Wet" : soil_moisture > 66.66 ? "Dry" : "Normal";
  lightLevel = lux < 807 ? "Low" : (lux > 808 && lux < 1614 ? "Medium" : "High");

  // Display and control system status
  displaySoilStatus();
  displayLightStatus();
  displayEnvironment();
  controlPump();

  Serial.println("------------------------");
  delay(3000);
  lcd.clear();
}
// ----------------------------------------

// Display soil moisture level and condition
void displaySoilStatus() {
  lcd.setCursor(0, 0);
  lcd.print("Soil        :");
  lcd.setCursor(13, 0);
  lcd.print(soilCondition);
  Serial.println("Soil Moisture      : " + String(soil_moisture, 2) + " % (" + soilCondition + ")");
}

// Display light intensity and LED indicator
void displayLightStatus() {
  lcd.setCursor(0, 1);
  lcd.print("Light       :");
  lcd.setCursor(13, 1);
  lcd.print(lightLevel);
  Serial.println("Light Intensity    : " + String(lux, 1) + " Lux (" + lightLevel + ")");

  if (lux < 807) {
    digitalWrite(LED_PIN, HIGH); // Turn on light
    Serial.println("Light Level Low - LED ON");
  } else {
    digitalWrite(LED_PIN, LOW);  // Turn off light
    Serial.println("Light Level OK - LED OFF");
  }
}

// Display temperature and humidity readings
void displayEnvironment() {
  lcd.setCursor(0, 2);
  lcd.print("Temp        :");
  lcd.setCursor(13, 2);
  lcd.print(temperature, 1);
  lcd.setCursor(19, 2);
  lcd.print("C");

  lcd.setCursor(0, 3);
  lcd.print("Humidity    :");
  lcd.setCursor(13, 3);
  lcd.print(humidity, 1);
  lcd.setCursor(19, 3);
  lcd.print("%");

  Serial.println("Temperature        : " + String(temperature, 1) + " °C");
  Serial.println("Humidity           : " + String(humidity, 1) + " %");
}

// Control water pump based on soil condition
void controlPump() {
  if (soil_moisture > 66.66) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Soil is DRY!");
    lcd.setCursor(0, 1);
    lcd.print("Dry and a pumping");
    digitalWrite(relayPin, HIGH);  // Turn on pump
    Serial.println("Soil is dry - Water Pump ON");
    delay(3000); // Wait to let the message be seen
    lcd.clear();
  } else {
    digitalWrite(relayPin, LOW);   // Turn off pump
    Serial.println("Soil moisture adequate - Pump OFF");
  }
}
// ----------------------------------------
