// This is the library for the TB6612 that contains the class Motor and all the
// functions
#include <SparkFun_TB6612.h>

#include <Wire.h>

// configure I2C address
#define ADDRESS 0

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
  
}



// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  
  uint8_t channel = Wire.read();    // receive channel
  int duty = Wire.read();    // receive duty cycle

  // I2C works with unsigned bytes so there is some transformation needed
  if (duty > 127) {
    duty = 256 - duty;
    duty *= -1;
  }
  
  Serial.print(F("* received channel: "));
  Serial.print(channel);
  Serial.print(F(", duty cycle: "));
  Serial.println(duty);  

  // set TEC output
  setTEC(channel, duty);

}


// set TEC output duty cycle
//
void setTEC(uint8_t channel, int duty) {

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
