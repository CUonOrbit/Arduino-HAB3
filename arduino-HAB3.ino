#include <Wire.h>
#include "SDCardModule.h"
#include "BMP280Module.h"
#include "config.h"

//
//unsigned long oneHour = 3600000UL;     // 1 hour in miliseconds
unsigned long oneHour = 20000UL;     // 10 seconds (TESTING)
unsigned long oneMinute = 60000UL; 
unsigned long startTime;
bool pulseStarted = false;
//

// LED blink timing
const int errorBlinkInterval = 500; // milliseconds
bool errorLedState = false;
unsigned long lastBlinkTime = 0;

bool isTerminating = false;
unsigned long terminationStart = 0;

// 5 Hz -> 200 ms interval for BMP280
const unsigned long SENSOR_INTERVAL_MS = 200UL; // 5Hz
unsigned long lastSensorMs = 0;

// flush every 5 seconds
const unsigned long FLUSH_INTERVAL_MS = 5000UL; 
unsigned long lastFlushMs = 0;

// Create BMP280 module instance
BMP280Module bmp280(0x76, 1013.25f, SENSOR_INTERVAL_MS);

void setup() {
  // Termination setup
  pinMode(5, OUTPUT);          // NiCr output
  startTime = millis(); // Start the 1-hour countdown

  // REMINDER:
  // Resetting or disconnecting/reconnecting power
  // restarts the 1-hour timer from zero.
  //

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  Serial.begin(115200);
  while (!Serial);      // remove when standalone

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

    // Periodic SD flush
    if (now - lastFlushMs >= FLUSH_INTERVAL_MS) {
      lastFlushMs += FLUSH_INTERVAL_MS;
      flushSD();
    }
//

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
//
    // Termination
    // unsigned long currentTime = millis();  
    // if((terminationStart == 0) && (currentTime > TERMINATION_TIME)) {
    //   logToSDCard("TERMINATING FLIGHT"); 
    //   digitalWrite(RELAY_PIN, HIGH); // Turn relay_pin 7 ON
    //   isTerminating = true;
    //   terminationStart = currentTime;
    // }
    // else if(terminationStart && currentTime - terminationStart > TERMINATION_CUT_TIME){
    //   logToSDCard("FLIGHT TERMINATION COMPLETE"); 
    //   digitalWrite(RELAY_PIN, LOW);  // Turn relay OFF
    //   isTerminating = false;
    // }
  }
}