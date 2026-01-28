#include <Wire.h>
#include "SDCardModule.h"
#include "BMP280Module.h"
#include "ThermistorModule.h"
#include "config.h"

// Termination
//unsigned long oneHour = 3600000UL;     // 1 hour in miliseconds
unsigned long oneHour = 20000UL;     // 10 seconds (TESTING)
unsigned long oneMinute = 60000UL; 
unsigned long startTime = 0;
bool pulseStarted = false;

// LED blink timing
const int errorBlinkInterval = 500; // milliseconds
bool errorLedState = false;
unsigned long lastBlinkTime = 0;

// 5 Hz -> 200 ms interval for BMP280
const unsigned long SENSOR_INTERVAL_MS = 200UL; // 5Hz
unsigned long lastSensorMs = 0;

// flush every 5 seconds
const unsigned long FLUSH_INTERVAL_MS = 5000UL; 
unsigned long lastFlushMs = 0;

// Create module instance
BMP280Module bmp280(0x76, 1013.25f, SENSOR_INTERVAL_MS);
ThermistorModule thermistor(ThermistorPin);

void setup() {
  // REMINDER:
  // Resetting or disconnecting/reconnecting power
  // restarts the 1-hour timer from zero.
  pinMode(RELAY_PIN, OUTPUT);          // NiCr output
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  Serial.begin(115200);
  //while (!Serial);      // remove when standalone

  delay(1000);
  Serial.println(F("BMP280 5Hz"));
  Serial.println(F("SD Logging"));

  // Track module initialization status
  bool sdOK = initSDCard(SD_CS_PIN);
  bool bmpOK = bmp280.begin();


  if (!sdOK) Serial.println(F("SD card initialization failed."));
  if (!bmpOK) Serial.println(F("BMP280 initialization failed."));


  Serial.print("SD Card status: ");
  Serial.println(sdOK ? "Success" : "Failed");

  Serial.print("BMP280 status: ");
  Serial.println(bmpOK ? "Success" : "Failed");

  
  if (!sdOK || !bmpOK) {
    errorLedState = true;
  }

  startTime = millis(); // Start the 1-hour countdown
  lastSensorMs = millis();
  lastFlushMs = millis();
}

void loop(){
  unsigned long now = millis();

  // Error LED handling
  if (errorLedState) {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  // 5Hz sensor logging 
  if (now - lastSensorMs >= SENSOR_INTERVAL_MS) {
    // Move the timestamp forward by exactly one interval
    lastSensorMs += SENSOR_INTERVAL_MS;

    // BMP280 data
    if (bmp280.update()) {                     // will be true whenever it takes a new sample
      const BMP280Reading &r = bmp280.getLastReading();

      if (!bmp280.isValid()) {                // checks the last reading's pressure against the range (300-1100 hPa)
        String barometerLog = String("Temp: ") + r.temperatureC +
                                "C, Pressure: " + r.pressureHpa +
                                "hPa (OOR), Altitude: " + r.altitudeM +       // flag it as out-of-range
                                "m (OOR)";  
        logToSDCard(barometerLog);
      } else {
        String barometerLog = String("Temp: ") + r.temperatureC +
                                "C, Pressure: " + r.pressureHpa +
                                "hPa, Altitude: " + r.altitudeM +
                                "m";
        logToSDCard(barometerLog);                        
      }
    } 

    float boardTempC = thermistor.readTemperatureC();
    if (thermistor.isValid(boardTempC)) {
      String tempLog = thermistor.getLogString(boardTempC);
      logToSDCard(tempLog);  // Logs to SD + Serial
      Serial.print("BoardTemp: "); 
      Serial.print(boardTempC, 1);
      Serial.println(F("°C"));
    } else {
      logToSDCard("BoardTemp: INVALID");
      Serial.println(F("BoardTemp: INVALID"));
    }

    // Vout = analogRead(ThermistorPin);
    // R2 = R1 * (1023.0 / (float)Vout - 1.0); // Voltage divider to find the second resistance
    // logR2 = log(R2);
    // Temp = (1.0 / (c1 + c2*logR2 + c3*logR2*logR2*logR2)); // Temperature in Kelvin
    // Temp = Temp - 273.15; // Converting to Celsius 

    // Serial.print("Temperature: "); 
    // Serial.print(Temp);
    // Serial.println("°C");

    // Periodic SD flush
    if (now - lastFlushMs >= FLUSH_INTERVAL_MS) {
      lastFlushMs += FLUSH_INTERVAL_MS;
      flushSD();
    }
  
    // --- Step 1: Wait 1 hour ---
    if (!pulseStarted && (millis() - startTime >= oneHour))  {
      logToSDCard("TERMINATING FLIGHT");
      digitalWrite(RELAY_PIN, HIGH);            // RELAY_PIN 5; Turn NiCr ON
      pulseStarted = true;              // Mark that the 1-minute pulse has begun
      startTime = millis();             // Reuse timer to measure the 1-minute duration
    }

    // --- Step 2: After pulse starts, run NiCr for 1 minute ---
    if (pulseStarted && (millis() - startTime >= oneMinute))  {
      digitalWrite(RELAY_PIN, LOW);                                            // Turn NiCr OFF after 1 minute
      logToSDCard("FLIGHT TERMINATION COMPLETE");
      // After this point, everything stays off permanently
    }
  }
}