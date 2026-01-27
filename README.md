HAB BMP280 Data Logger
[
[

High Altitude Balloon (HAB) telemetry payload featuring 5Hz BMP280 logging to SD card with autonomous cut-down at 30-minute timeout.

Features
5Hz BMP280 logging (temperature, pressure, altitude)

32GB SD card data storage with timestamps

Termination: 30 minutes flight time

Standalone operation (9V lithium battery)

Out-of-range detection (>9km altitude warning)

Arduino Uno R4 compatible

Hardware Requirements

Required:
• Arduino Uno R4
• BMP280 barometer (I2C)
• SD card adapter + 32GB microSD (FAT32)
• 9V lithium battery
• Relay module (for cut-down termination)

Wiring Diagram
Component	VCC	GND	SCL	SDA	CS	SCK	MOSI	MISO
BMP280	5V	GND	A5	A4	-	-	-	-
SD Card	5V	GND	-	-	D4	D13	D11	D12
Relay	-	-	-	-	D7	-	-	-
Power: 9V battery → Arduino 9V DC power barrel jack for standalone operation.

Pin Configuration is in config.h

Installation
Install Libraries (Arduino IDE Library Manager):

-  Adafruit BMP280 Library
-  Adafruit Unified Sensor
-  SD (built-in)
Upload Arduino-HAB3.ino to Arduino Uno R4

Insert formatted microSD card (FAT32)

Standalone: Connect 9V lithium to Arduino 9V DC power barrel jack
PC Test: USB power only

Expected Output
Serial Monitor (115200 baud):

BMP280 status: Success
SD Card status: Success

SD Card log.txt:
0:1:23 -> Temp: 22.5C, Pressure: 1002.3hPa, Altitude: 75m
0:1:23 -> Temp: 22.6C, Pressure: 1002.1hPa, Altitude: 78m
...
0:30:00 -> TERMINATING FLIGHT
0:32:00 -> FLIGHT TERMINATION COMPLETE

Operation
Power on → Both modules initialize

5Hz logging → BMP280 data to SD card

Every 5s → SD buffer flush (data safety)

30min → Relay activates (D7 HIGH)

+2min → Relay deactivates (cut-down complete)

Status Indicators
LED	Pin	Meaning
-

Power Consumption
Arduino Uno R4: ~80mA
BMP280: ~1mA
SD Card: ~30mA (avg)
Total: ~110mA

9V Lithium (1000mAh): 5~9 hours flight time

Troubleshooting
Symptom	Cause	Fix
"BMP280 init failed"	Ensure I2C address (0x76) and pin connections
"SD init failed"	Wrong CS pin	Verify D4 connection
Hangs at startup	while(!Serial)	Comment out for standalone

Development Status
✅ BMP280 5Hz logging
✅ SD card storage
✅ Autonomous termination
✅ Standalone operation
