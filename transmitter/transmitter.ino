/*************************************************** 
  This is an example for the HTU21D-F Humidity & Temp Sensor

  Designed specifically to work with the HTU21D-F sensor from Adafruit
  ----> https://www.adafruit.com/products/1899

  These displays use I2C to communicate, 2 pins are required to  
  interface
 ****************************************************/

#include <Wire.h>
#include <SFE_BMP180.h>
#include "Adafruit_HTU21DF.h"

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
Adafruit_HTU21DF htu = Adafruit_HTU21DF();
SFE_BMP180 pressure;
typedef struct {
  byte key;
  double value;
} key_value_struct;
key_value_struct weather_element;
typedef struct {
  double temp;
  double humidity;
  double pressure;
  double voltage;
  long unsigned int uptime;
} weather_struct;
weather_struct weather;

RF24 radio(7,8);
const uint64_t PIPE = 0xAFAFAF4422LL;
byte addresses[][6] = {"1Node","2Node"};


long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

void setup() {

  delay(1000);
  weather.temp = 0;
  weather.humidity = 0;
  weather.pressure = 0;

  radio.begin();
  radio.setDataRate( RF24_1MBPS);
  radio.setChannel(99);
  radio.setPALevel(RF24_PA_MAX);
  radio.openWritingPipe(addresses[1]);

    

  Serial.begin(115200);
  
  if (!htu.begin()) {
    while (1);
  }
  if (!pressure.begin())
  {
    Serial.println("BMP180 init fail\n\n");
    while(1); // Pause forever.
  }
}


void loop() {
      weather_element.key = 0x20;
    weather_element.value = htu.readHumidity();
  Serial.println(weather_element.value);
    radio.flush_tx();
    radio.write( &weather_element, sizeof(key_value_struct) );
    delay(200);
  delay(1000);
  
  char status;
  double T,P,p0,a;

  status = pressure.startTemperature();
  if (status != 0) {
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0) {
          weather_element.key = 0x10;
    weather_element.value = T;
  Serial.println(weather_element.value);
    radio.flush_tx();
    radio.write( &weather_element, sizeof(key_value_struct)) ;
    delay(200);
  delay(1000);
      status = pressure.startPressure(0);
      if (status != 0) {
        delay(status);
        status = pressure.getPressure(P,T);
        if (status != 0) {
              weather_element.key = 0x40;
              weather_element.value = P*0.7500 - 750;
              
  Serial.println(weather_element.value);
              radio.flush_tx();
              radio.write( &weather_element, sizeof(key_value_struct) );
                delay(200);
  delay(1000);
        }
      }
    }
  }

  delay(200);
  delay(1000);
  weather_element.key = 0x80;
  weather_element.value = readVcc()/1000.0;
  radio.flush_tx();
  radio.write( &weather_element, sizeof(key_value_struct) );
  Serial.print(readVcc());
  Serial.println(" v_bat");


 
  delay(5000);  // Pause for 1 minute.
}
