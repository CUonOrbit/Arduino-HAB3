// config.h - Pin Definitions and Hardware Settings

#pragma once
    

// I2C Pins - fixed on Uno
#define SDA_PIN    A4           
#define SCL_PIN    A5         

// SD Card Module (SPI Pins - fixed on Uno)
#define SD_CS_PIN         4           
// (MOSI=11, MISO=12, SCK=13 are fixed in SPI)

#define RELAY_PIN         5

#define THERMISTOR_PIN    A0

//#define TERMINATION_TIME 1800000UL    // 30 minutes
//#define TERMINATION_CUT_TIME 120000   // 2 minutes (relay burn)
//#define TERMINATION_HEIGHT   30000  // In metres