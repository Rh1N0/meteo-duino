
/*
* Getting Started example sketch for nRF24L01+ radios
* This is a very basic example of how to send data from one node to another
* Updated: Dec 2014 by TMRh20
*/

#include <SPI.h>
#include "RF24.h"

#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// pin 3 - Serial clock out (SCLK)
// pin 4 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 6 - LCD chip select (CS)
// pin 2 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(A4, A3, A2, A1, A0);

/****************** User Config ***************************/
/***      Set this radio as radio number 0 or 1         ***/
bool radioNumber = 0;

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 7 & 8 */
RF24 radio(7,8);
/**********************************************************/

byte addresses[][6] = {"1Node","2Node"};
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
bool role = 0;

void setup() {
    // Инициализация дисплея
  display.begin();
//
//  // Очищаем дисплей
  display.clearDisplay();
  display.setTextSize(0);
  display.setCursor(0, 0);
  display.println("Waiting...");
  display.display();
//
//  // Устанавливаем контраст
  display.setContrast(50);
  delay(1000);
  Serial.begin(115200);
  Serial.println("Starting receiver");
  radio.begin();

  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(99);
  radio.setDataRate(RF24_1MBPS);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  if(radioNumber){
    radio.openWritingPipe(addresses[1]);
    radio.openReadingPipe(1,addresses[0]);
  }else{
    radio.openWritingPipe(addresses[0]);
    radio.openReadingPipe(1,addresses[1]);
  }
  
  radio.startListening();
}

void sendData() {
      unsigned long got_time = 123ul;
      radio.stopListening();                                        // First, stop listening so we can talk   
      radio.write( &got_time, sizeof(unsigned long) );              // Send the final one back.      
      radio.startListening();       
}

void loop() {
  
    
    
    if( radio.available()){
                                                                    // Variable for the received timestamp
      while (radio.available()) {                                   // While there is data ready
        radio.read( &weather_element, sizeof(key_value_struct) );             // Get the payload
      }
      switch(weather_element.key) {
        case 0x10: weather.temp = weather_element.value; break;
        case 0x20: weather.humidity = weather_element.value; break;
        case 0x40: weather.pressure = weather_element.value+750; break;
        case 0x80: weather.voltage = weather_element.value; break;
        case 0x88: weather.uptime = weather_element.value; break;
        default: break;
      }
      
      display.clearDisplay();
      display.setContrast(50);
      display.setCursor(0, 0);
      // Устанавливаем цвет текста
      display.setTextColor(BLACK);
      // Устанавливаем размер текста
      // Выводим текст
      if (weather.temp < 0) display.print("-");
      display.print(round(weather.temp));
      display.println("*C");
      
      display.print(round(weather.humidity));
      display.println("%");
      
      display.print(round(weather.pressure));
      display.println(" mmPc");
      
      
      display.print(weather.voltage);
      display.println(" V");
      
      display.display();

      sendData();
      Serial.print("Temp = ");
      Serial.println(weather.temp);
      Serial.print("Pressure = ");
      Serial.println(weather.pressure);
      Serial.print("Humidity = ");
      Serial.println(weather.humidity); 
      Serial.print("Voltage = ");
      Serial.println(weather.voltage); 
    }
} // Loop

