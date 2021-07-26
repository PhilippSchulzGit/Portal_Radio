/*
Portal Radio by Urikal (https://www.thingiverse.com/thing:3722053,
using the original model by Cerbs (https://www.thingiverse.com/thing:1431366)

- install the movingAvg Lib using the Arduino Lib Manager
  - info: https://www.arduinolibraries.info/libraries/moving-avg
- install the Attiny Core package using the Arduino Board Manager and use to flash the Attiny85
  - settings:
    Attiny 85
    16 Mhz PLL
    BOD 2.7V
    EEPROM not retained
- wiring:
  - LDR at ATTiny A3
  - JQ K1 at ATTiny D1
*/


#include <movingAvg.h> 
movingAvg ldr(40);

// Variables
unsigned long l1=0;
byte startup=60;

void setup() {
  pinMode(1, INPUT);
  pinMode(A3, INPUT);
  ldr.begin();
}

void loop() {
    int adc_raw = analogRead(A3);
         
    // Input-Spannung am ADC berechnen
    float voltage = 5.0 * (adc_raw / 1024.0);
    // Widerstand des Fotoresistors im Spannungsteiler berechnen
    float resistance = (10.0 * 5.0) / voltage - 10.0;
    // Beleuchtungsstärke in lux berechnen    
    float illuminance = 255.84 * pow(resistance, -10/9);
    float illuminance_avg = ldr.reading(illuminance);

    // z-score: z = (x - mean)/(standard deviation)
    float illuminance_z = ((illuminance - illuminance_avg) / 40);

    if (startup > 0) {
      startup = startup-1;      
    } 
    else 
    {
      // 300 sekunden, nur alle 5 minuten ausloesen
      if ((illuminance_z > 0.1) && dly(l1,300000)){
        pinMode(1, OUTPUT);
        delay(250);
        pinMode(1, INPUT);
        l1 = millis();      
      }      
    }
}

boolean dly(unsigned long time, unsigned int diff){
  // x 1000 fuer sekunden
  if (((time + diff)<=millis()) or (time==0))
  {
    return true;
  }
  else
  {
    return false;
  }
}
