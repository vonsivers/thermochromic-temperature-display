// This is the library for the TB6612 that contains the class Motor and all the
// functions
#include <SparkFun_TB6612.h>

#include <Wire.h>

// configure I2C address
#define ADDRESS 1

// Pins for all inputs, keep in mind the PWM defines must be on PWM pins
#define PWMA_1 3
#define AIN2_1 2
#define AIN1_1 4
#define STBY_1 7
#define BIN1_1 8
#define BIN2_1 10
#define PWMB_1 5

#define PWMA_2 6
#define AIN2_2 11
#define AIN1_2 12
#define STBY_2 13
#define BIN1_2 A0
#define BIN2_2 A1
#define PWMB_2 9


// these constants are used to allow you to make your motor configuration 
// line up with function names like forward.  Value can be 1 or -1
const int offsetA_1 = 1;
const int offsetB_1 = 1;
const int offsetA_2 = 1;
const int offsetB_2 = 1;

// TEC duty cycle for cooling/heating (-255 ... 255)
// 
const int duty_cool_1 = -40;
const int duty_cool_2 = -30;    
const int duty_heat_1 = 30; 
const int duty_heat_2 = 20; 

// duration (ms) of first cooling/heating phase
const unsigned long duration_cool = 35000;
const unsigned long duration_heat = 25000;

// current dutycycle of TEC 
int stateTEC[4] = {0, 0, 0, 0};
// last dutycycle of TEC 
int lastStateTEC[4] = {0, 0, 0, 0};

// time (ms) since last change of TEC dutycycle
unsigned long lastMillis[4] = {0, 0, 0, 0};


// Initializing motors.  The library will allow you to initialize as many
// motors as you have memory for.  If you are using functions like forward
// that take 2 motors as arguements you can either write new functions or
// call the function more than once.
Motor TEC1 = Motor(AIN1_1, AIN2_1, PWMA_1, offsetA_1, STBY_1);
Motor TEC2 = Motor(BIN1_1, BIN2_1, PWMB_1, offsetB_1, STBY_1);
Motor TEC3 = Motor(AIN1_2, AIN2_2, PWMA_2, offsetA_2, STBY_2);
Motor TEC4 = Motor(BIN1_2, BIN2_2, PWMB_2, offsetB_2, STBY_2);


void setup() {
  Serial.begin(9600);

  Wire.begin(ADDRESS);                // join i2c bus with address
  Wire.onReceive(receiveEvent); // register event

  TEC1.standby();
  TEC2.standby();
  TEC3.standby();
  TEC4.standby();

}

void loop() {

  // wait some time
  delay(100);
  
  // iterate through TECs
  for(int i=0; i<4; ++i) {
    // check if TEC is cooling/heating
    if(stateTEC[i] < 0) {
      // check if cooling duration has expired
      if( (millis()-lastMillis[i]) > duration_cool) {
        Serial.print(F("* TEC"));
        Serial.print(i+1);
        Serial.print(F(" was cooling for more than "));
        Serial.println(duration_cool);
        setTEC(i,0);
      }
    }
    else if(stateTEC[i] > 0) {
      // check if heating duration has expired
      if( (millis()-lastMillis[i]) > duration_heat) {
        Serial.print(F("* TEC"));
        Serial.print(i+1);
        Serial.print(F(" was heating for more than "));
        Serial.println(duration_heat);
        setTEC(i,0);
      }
    }
  }
}



// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  
  uint8_t channel = Wire.read();    // receive channel
  int state = Wire.read();    // receive state
  
  Serial.print(F("* received channel: "));
  Serial.print(channel);
  Serial.print(F(", state: "));
  Serial.println(state);  

  int duty;
  switch (state) {
    case 0:
      // if TEC was cooling before
      if(lastStateTEC[channel]<0) {
        duty = duty_cool_2;
      }
      else {
        duty = duty_cool_1;
      }
      break;
    case 1:
      // if TEC was heating before
      if(lastStateTEC[channel]>0) {
        duty = duty_heat_2;
      }
      else {
        duty = duty_heat_1;
      }
      break;
  }

  // set TEC output
  setTEC(channel, duty);

  // update last state
  lastStateTEC[channel] = duty;
}


// set TEC output duty cycle
//
void setTEC(uint8_t channel, int duty) {

  // do not cool when TEC is already off
  //if( (stateTEC[channel] == 0) && (duty == duty_cool_1) ) return;

  // do not heat when TEC is already heating
  //if( (stateTEC[channel] == duty_heat_1) && (duty == duty_heat_1) ) return;
  
  stateTEC[channel] = duty;    // update TEC state
  lastMillis[channel] = millis();

/*
  Serial.print(F("** Update TEC"));
  Serial.print(channel+1);
  Serial.print(F(" state to "));
  Serial.print(state); 
  Serial.print(F(" millis to "));
  Serial.println(lastMillis[channel]);
  */

 switch (channel) {
      case 0:
            TEC1.drive(duty);
            Serial.print("*** TEC1 duty cycle set to ");
            break;
      case 1:
            TEC2.drive(duty);
            Serial.print("*** TEC2 duty cycle set to ");
            break;
      case 2:
            TEC3.drive(duty);
            Serial.print("*** TEC3 duty cycle set to ");
            break;
      case 3:
            TEC4.drive(duty);
            Serial.print("*** TEC4 duty cycle set to ");
            break;
     }
 
  Serial.println(duty);
  
}
