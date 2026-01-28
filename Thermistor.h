#ifndef THERMISTORMODULE_H
#define THERMISTORMODULE_H

#include <math.h>  // For log()

class ThermistorModule {
private:
  int pin;
  float R1;  // 10K fixed resistor
  float c1, c2, c3;  // Steinhart-Hart coefficients
  
public:
  ThermistorModule(int thermistorPin, float r1 = 10000.0f) 
    : pin(thermistorPin), R1(r1),
      c1(1.009249522e-03f), c2(2.378405444e-04f), c3(2.019202697e-07f) {}
  
  float readTemperatureC() {
    int Vout = analogRead(pin);
    if (Vout == 0 || Vout >= 1023) return NAN;  // Invalid reading
    
    float R2 = R1 * (1023.0f / (float)Vout - 1.0f);
    float logR2 = log(R2);
    float tempK = 1.0f / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2);
    return tempK - 273.15f;  // Kelvin → Celsius
  }
  
  String getLogString(float tempC) {
    return "BoardTemp: " + String(tempC, 1) + "°C";
  }
  
  bool isValid(float tempC) {
    return !isnan(tempC) && tempC > -50.0f && tempC < 150.0f;
  }
};

#endif
