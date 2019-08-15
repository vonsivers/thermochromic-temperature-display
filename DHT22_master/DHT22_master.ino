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

// time (s) between sensor readouts and duration of heating and cooling periods
// temp read out -> delay_heat -> delay_cool -> delayRead -> hum read out -> delay_heat -> delay_cool -> delay_read ...
const unsigned long delayRead = 35;
const unsigned long duration_heat = 25;
const unsigned long duration_cool = 0;

// TEC duty cycle (-255 .. 255) for heating/cooling
const int duty_heat = 30;
const int duty_cool = -30;

// current state of TEC
// stateTEC[address][channel]
int stateTEC[5][4] = {{0}};

float humidity;         // humidity
float temperature;      // temperature

uint8_t digit[2];         // digits to display

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
//  IIIIIICC    
  0b00001000,    // A -> 2,0
  0b00000110,    // B -> 1,2
  0b00000111,    // C -> 1,3
  0b00001100,    // D -> 3,0
  0b00000101,    // E -> 1,1
  0b00000100,    // F -> 1,0
  0b00001010     // G -> 2,2
  };

void setup() {
  Serial.begin(9600);

  Wire.begin(); // join i2c bus (address optional for master)

  dht.begin();
  
  delay(5000);
}

void loop() {
  
  /*
  temperature = random(18,33);
  humidity = random(20,80);
  
  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(temperature);
  Serial.println(F("°C "));
  */

  readDHT();
  displayTemp();
  delay(delayRead*1000);
  displayHum();
  delay(delayRead*1000);
  
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
  int temp = round(temperature);
  displayNum(temp);
  displayDeg();
  delay(duration_heat*1000);
  stopHeating();
  delay(duration_cool*1000);
  stopCooling();
}

void displayHum() {
  int hum = round(humidity);
  displayNum(hum);
  displayPer();
  delay(duration_heat*1000);
  stopHeating();
  delay(duration_cool*1000);
  stopCooling();
}


// displays 2 digit number
//
void displayNum(int number) {
  
  Serial.print(F("* displaying number "));
  Serial.println(number);

  // separate digits
  getDigits(number);

  // display both digits
  displayDigit(0);
  displayDigit(1);
}

// display first (dig=1) or second (dig=0) digit
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
    int duty;
    
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
      duty = duty_heat;
    }
    else {
      duty = duty_cool;
    }

    // separate I2C address and channel nr.
    uint8_t address = addressCh >> 2;
    uint8_t channel = addressCh & 0x3;

    Serial.print(F("***** I2C address "));
    Serial.print(address);
    Serial.print(F(", channel "));
    Serial.println(channel);
    
    // send data
    sendData(address, channel, duty);

   // go to next bit
    seg = seg >> 1; 
  }
}

// displays ° symbol
//
void displayDeg() {
  sendData(0, 0, duty_heat/5);    // dot 1, more sensitive foil
  sendData(0, 1, duty_cool);      // dot 2, normal foil
  //sendData(0, 2, duty_cool*2);   // dash lower TEC, more sensitive foil, bad thermal coupling!->disabled
  sendData(0, 3, duty_cool*2);   // dash upper TEC, more sensitive foil
}

// displays % symbol
//
void displayPer() {
  sendData(0, 0, duty_heat/6); 
  sendData(0, 1, duty_heat); 
  sendData(0, 2, duty_heat/7);
  sendData(0, 3, duty_heat/6);
}

// stop heating
//
void stopHeating() {
  for(int address=0; address<5; address++) {
    for(int channel=0; channel<4; channel++) {
      if(stateTEC[address][channel]>0) sendData(address,channel,0);
    }
  }
}

// stop cooling
//
void stopCooling() {
  for(int address=0; address<5; address++) {
    for(int channel=0; channel<4; channel++) {
      if(stateTEC[address][channel]<0) sendData(address,channel,0);
    }
  }
}

// stop cooling TEC with bad coupling
//
void stopCoolingBadTEC() {
  if(stateTEC[0][2]<0) sendData(0,2,0);
}

// transmitts data via I2C
//
void sendData(uint8_t address, uint8_t channel, int duty) {
 Wire.beginTransmission(address); // transmit to device 
  Wire.write(channel); 
  Wire.write(duty);              
  Wire.endTransmission();    // stop transmitting

  Serial.print(F("****** Sending to I2C address: "));
  Serial.print(address);
  Serial.print(F(", ch: "));
  Serial.print(channel);
  Serial.print(F(", duty cycle: "));
  Serial.println(duty);

  // update TEC state
  stateTEC[address][channel] = duty;
}

// get individual digits of two digit number
//
void getDigits(int number) {
  if(number>99) return;
  digit[0] = number % 10;
  digit[1] = number / 10;

  Serial.print(F("* digit 1: "));
  Serial.print(digit[1]);
  Serial.print(F(", digit 0: "));
  Serial.println(digit[0]);
}
