# Smart Agriculture Monitoring System

ESP32-based IoT sensor node that monitors soil moisture, ambient light,
temperature, humidity and tank/rain conditions, displays live readings on an
LCD, and automatically drives a water pump based on soil condition.

Built and simulated in [Wokwi](https://wokwi.com/).

## Hardware

| Component | Purpose | ESP32 Pin |
|---|---|---|
| DHT22 | Temperature & humidity | D15 |
| FC-28 soil moisture sensor *(simulated with a potentiometer — see note below)* | Soil moisture / dryness | D34 |
| LDR (photoresistor module) | Ambient light level | D35 |
| DS1307 RTC | Real-time clock, timestamps readings | I2C (D21/D22) |
| 20x4 I2C LCD | Live status display | I2C (D21/D22) |
| Relay module | Drives the water pump | D18 |
| Orange LED | Pump-on indicator (via relay COM) | — |
| Red LED + 220Ω resistor | Low-light indicator | D33 |

All I2C devices (LCD, RTC) share the same SDA/SCL bus.

### Why a potentiometer instead of a real soil sensor?

Wokwi has no built-in FC-28 component, since it can't simulate "a probe in
soil." The potentiometer stands in for it: turning its knob changes the
voltage on D34 exactly the way the real FC-28 would when soil gets wetter or
drier. The firmware reads that pin identically either way — swapping the
potentiometer for a real FC-28 probe on real hardware requires **no code
changes**, only the physical part swap.

## Software

- **Language / framework:** Arduino framework (C++), built for ESP32
- **Libraries used** (see `libraries.txt`):
  - `LiquidCrystal I2C` — LCD driver
  - `DHT sensor library` (Adafruit) — temperature/humidity
  - `RTClib` — DS1307 real-time clock
- **Custom driver:** `FC28.h` / `FC28.cpp` — reads the soil sensor's analog
  pin and converts the raw 12-bit ADC value (0–4095) linearly to a 0.0–100.0
  percentage.

### Soil reading convention (important)

`getSoilMoisture()` returns a **higher percentage for drier soil**, not
wetter — this matches how the sensor is wired as a voltage divider in this
project. The pump logic in `main.ino` is written against this convention:

```cpp
if (soil_moisture > 66.66) {
  // pump ON — soil is dry
}
```

This is documented in `FC28.h` so it isn't mistaken for a bug.

### Light level calculation

The LDR module's analog output is converted using the ESP32's actual ADC
characteristics (12-bit resolution, ~3.3V reference) and the standard
photoresistor resistance/lux relationship, then bucketed into Low / Medium /
High for the LCD and serial log.

## How it works

1. On boot, the system initializes Serial, all sensors, the LCD, and checks
   the RTC is present (halts with an LCD message if not — the RTC is treated
   as a hard requirement since the system timestamps every reading).
2. Every loop cycle (~3s):
   - Reads DHT22 (temp/humidity), LDR (light), and the soil sensor
   - Classifies soil as Wet / Normal / Dry and light as Low / Medium / High
   - Updates the LCD with all four readings
   - Logs the same data to Serial, timestamped by the RTC
   - Turns the pump (via relay) ON when soil is classified Dry, OFF otherwise
   - Turns the low-light LED ON when light is Low

## Files

| File | Purpose |
|---|---|
| `main.ino` | Main sketch — setup, loop, display, pump control |
| `FC28.cpp` / `FC28.h` | Soil moisture sensor driver |
| `diagram.json` | Wokwi circuit/wiring definition |
| `libraries.txt` | Wokwi library manifest |

## Running the simulation

1. Open the project in Wokwi (or recreate the wiring per `diagram.json`).
2. Start the simulation.
3. Turn the potentiometer to simulate changing soil moisture and observe:
   - The LCD's "Soil" status updating (Wet / Normal / Dry)
   - The pump (relay + orange LED) switching on when soil reads Dry
4. Adjust the DHT22's temperature/humidity sliders and the LDR's light level
   to confirm the display and low-light LED respond accordingly.

## Known limitations

- The soil sensor is simulated (potentiometer), not a physical FC-28 probe —
  see note above.
- No Wi-Fi/MQTT connectivity in this version — all data is local (LCD +
  Serial only). If remote logging/control is required, that would need to be
  added separately (ESP32 Wi-Fi + an MQTT client), which was present in an
  earlier version of this project but is not part of this build.
- If the RTC is not detected at boot, the system halts (`while(1)`) rather
  than continuing without timestamps — this is intentional, not an
  unhandled failure.

## Author

Alyx — UCT Internship submission.
