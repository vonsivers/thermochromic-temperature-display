// Example testing sketch for various DHT humidity/temperature sensors
// Written by ladyada, public domain

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"
#include <Wire.h>

#define DHTPIN 2     // Digital pin connected to the DHT sensor

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

// time (s) between sensor readouts
const unsigned long delayRead = 60;
// time (s) for changing between temp and humidity display
//const unsigned long delayHum = 20;

float humidity=20;         // humidity
float temperature=10;      // temperature

uint8_t digit[2];

//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D
const uint8_t digitToSegment[] = {
 // XGFEDCBA
  0b00111111,    // 0
  0b00000110,    // 1
  0b01011011,    // 2
  0b01001111,    // 3
  0b01100110,    // 4
  0b01101101,    // 5
  0b01111101,    // 6
  0b00000111,    // 7
  0b01111111,    // 8
  0b01101111,    // 9
  };

// I2C adress and channel for segments of digit 0
// first two bits are channel nr., last 6 bits are I2C address
  const uint8_t segmentZeroToAddressCh[] = {
 // IIIIIICC
  0b00001001,    // A -> 2,1
  0b00010010,    // B -> 4,2
  0b00010011,    // C -> 4,3
  0b00001101,    // D -> 3,1
  0b00010001,    // E -> 4,1
  0b00010000,    // F -> 4,0
  0b00001011     // G -> 2,3
  };

  // I2C adress and channel for segments of digit 1
// first two bits are channel nr., last 6 bits are I2C address
  const uint8_t segmentOneToAddressCh[] = {
 // IIIIIICC
    0b00000100,    // A -> 1,0
    0b00000101,    // B -> 1,1
    0b00000110,    // C -> 1,2
    0b00000111,    // D -> 1,3
    0b00001000,    // E -> 2,0
    0b00001001,    // F -> 2,1
    0b00001010     // G -> 2,2
    
  //0b00001000,    // A -> 2,0
  //0b00000110,    // B -> 1,2
  //0b00000111,    // C -> 1,3
  //0b00001100,    // D -> 3,0
  //0b00000101,    // E -> 1,1
  //0b00000100,    // F -> 1,0
  //0b00001010     // G -> 2,2
  };

void setup() {
  Serial.begin(9600);

  Wire.begin(); // join i2c bus (address optional for master)

  dht.begin();
  
  delay(5000);
}

void loop() {

  
  //temperature = random(10,99);
  //humidity = random(10,99);
  
  if (temperature == 110) temperature = 10;
  if (humidity == 100) humidity = 0;
  //Serial.print(F("Humidity: "));
  //Serial.print(humidity);
  //Serial.print(F("%  Temperature: "));
  Serial.print(temperature);
  Serial.println(F("°C "));

  //readDHT();
  displayTemp();
  //delay(delayHum*1000);
  delay(delayRead*1000);
  displayHum();
  delay(delayRead*1000);

  humidity += 20;
  temperature += 20;
  
}

void readDHT() {

  do {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
    temperature = dht.readTemperature();
  }
  while (isnan(humidity) || isnan(temperature));

  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(temperature);
  Serial.println(F("°C "));
  
}

void displayTemp() {
  int temp = (int)(temperature);
  displayNum(temp);
  displayDeg();
}

void displayHum() {
  int hum = (int)(humidity);
  displayNum(hum);
  displayPer();
}


// displays 2 digit number
//
void displayNum(int number) {
  
  Serial.print(F("* displaying number "));
  Serial.println(number);

  // convert number to digits
  getDigits(number);

  displayDigit(0);
  displayDigit(1);
}

// display digit 0 or 1
//
void displayDigit(uint8_t dig) {
  
  // get bit array for digit
  uint8_t seg;
  if (dig == 0) {
    seg = digitToSegment[digit[0]];
    Serial.print(F("** displaying digit "));
    Serial.println(digit[0]);
  }
  else {
    if(digit[1]==0) {  // no leading zero
      seg=0;
      Serial.println(F("** leading zero: turn all segments off "));
    }
    else {
      seg = digitToSegment[digit[1]];
      Serial.print(F("** displaying digit "));
      Serial.println(digit[1]);
    }
  }
  Serial.print(F("*** segments as bit array "));
  Serial.println(seg);
 
  // iterate through bit array
  for(uint8_t i = 0; i < 7; i++) {
    
    uint8_t addressCh;
    uint8_t isActive = 0;
    
    // get I2C address and channel
    if (dig == 0) {
      addressCh = segmentZeroToAddressCh[i];
    }
    else {
      addressCh = segmentOneToAddressCh[i];
    }
    Serial.print(F("**** combined I2C address and channel "));
    Serial.println(addressCh);
    
    // check if segment is active
    if (seg & 0x01) {
      isActive = 1;
    }

    // separate I2C address and channel nr.
    uint8_t address = addressCh >> 2;
    uint8_t channel = addressCh & 0x3;

    Serial.print(F("***** I2C address "));
    Serial.print(address);
    Serial.print(F(", channel "));
    Serial.println(channel);
    
    // send data
    sendData(address, channel, isActive);
   
    seg = seg >> 1; 
  }
}

// displays ° symbol
//
void displayDeg() {
  sendData(0, 0, 1); 
  sendData(0, 1, 0);
  sendData(0, 2, 0);
  sendData(0, 3, 0);
}

// displays % symbol
//
void displayPer() {
  sendData(0, 0, 1); 
  sendData(0, 1, 1);
  sendData(0, 2, 1);
  sendData(0, 3, 1);
}

// transmitts data via I2C
//
void sendData(uint8_t address, uint8_t channel, uint8_t state) {
 Wire.beginTransmission(address); // transmit to device 
  Wire.write(channel); 
  Wire.write(state);              
  Wire.endTransmission();    // stop transmitting

  Serial.print(F("****** Sending ... ch: "));
  Serial.print(channel);
  Serial.print(F(", state: "));
  Serial.println(state);
}

void getDigits(int number) {
  if(number>99) return;
  digit[0] = number % 10;
  digit[1] = number / 10;

  Serial.print(F("* digit 1: "));
  Serial.print(digit[1]);
  Serial.print(F(", digit 0: "));
  Serial.println(digit[0]);
}
