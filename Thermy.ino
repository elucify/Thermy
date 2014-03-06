#include <OneWire.h>
#include <DallasTemperature.h>
#include <Thermometer.h>
#include <TempController.h>
#include <LedControl.h>

//
// TODO: Protocol to register a specific probe to channel A or B:
// Disconnect all probes but one
// Switch to left or right
// Hold down both buttons 2 seconds
// Thereafter that probe will be recognized as A or B.
// Save in EPROM
// Remember to remove from other channel if already there.
//
// TODO: Minimize number of methods in DisplayBoard class to save memory--< 3K flash left!
//
// TODO: Create TempController class? Maybe easier to organize thinking.
//

// This Arduino sketch reads DS18B20 "1-Wire" digital
// temperature sensors.
// Tutorial:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-tutorial.html


DeviceAddress da_thermometer_a = { 0x28, 0xFC, 0x74, 0xC9, 0x02, 0x00, 0x00, 0x86 };
DeviceAddress da_thermometer_b = { 0x28, 0x6B, 0xDF, 0xDF, 0x02, 0x00, 0x00, 0xC0 };

// TODO: Keep these in ROM, create channel assignment protocol
Thermometer thermometer_a(da_thermometer_a);
Thermometer thermometer_b(da_thermometer_b);

// Number of 
#define HOLDPERIOD0 333

/* we always wait a bit between updates of the display */
int count = 0;
int holdcount = 0;
int holdperiod = HOLDPERIOD0;
byte state, prevstate;
float temp_a, temp_b;
float tolerance_a, tolerance_b;
float ref_temp_a, ref_temp_b;
static DisplayBoard displayBoard;

// Time per loop
#define DELAYTIME 2

#define STATE_READ 0
#define STATE_WAIT_BUTTON_UP 1
#define STATE_WAIT_BUTTON_DOWN 2
#define STATE_COUNT_DOWN 3
#define STATE_COUNT_UP 4

#define WHICH_BUTTON_PUSHED (displayBoard.isButtonPushed(BUTTON_DOWN) ? "down" : (displayBoard.isButtonPushed(BUTTON_UP) ? "up" : "none"))

void printButtons() {
    Serial.print("UP = ");
    Serial.println(displayBoard.isButtonPushed(BUTTON_UP));
    Serial.print("DOWN = ");
    Serial.println(displayBoard.isButtonPushed(BUTTON_DOWN));
    Serial.print("RIGHT = ");
    
    Serial.print("Switch is ");
    if (displayBoard.isSwitchRight()) {
      Serial.println("RIGHT");
    }
    
    if (displayBoard.isSwitchLeft()) {
      Serial.println("LEFT");
    }
    
    if (displayBoard.isSwitchCenter()) {
      Serial.println("CENTER");
    }
}

void initializeSerial() {
    // Serial
    Serial.begin(9600);
    while (!Serial) {
    }
}

void updateDisplay(float a, float b,
   boolean cold_a = false,
   boolean hot_a = false,
   boolean cold_b = false,
   boolean hot_b = false)
{
  displayBoard.temp(CHANNEL_A, a);
  displayBoard.temp(CHANNEL_B, b);
  
  displayBoard.hot(CHANNEL_A, hot_a);
  displayBoard.hot(CHANNEL_B, hot_b);

  displayBoard.cold(CHANNEL_A, cold_a);
  displayBoard.cold(CHANNEL_B, cold_b);
}

void initializeDisplayBoard() {
    
    printButtons();
        
    state = STATE_WAIT_BUTTON_DOWN;
    prevstate = 0xff;
    count = 0;
    holdcount = 1000 / DELAYTIME; // After 2 seconds, start counting fast
    holdperiod = HOLDPERIOD0;
    temp_a = temp_b = 0.0;
    tolerance_a = tolerance_b = 0.5;
    ref_temp_a = ref_temp_b = 48.0;
    updateDisplay(temp_a, temp_b, false, false, false, false);    
}

void readTemperatures() {
  float a_temp = thermometer_a.getTemperature();
  if (a_temp > -100.0) {
     temp_a = thermometer_a.getTemperature();
  } else {
     Serial.println("A: get temp failed");
  }
//  sensors.requestTemperatures();
//  readTemperature(thermometer_a, 'A', temp_a);
}

void setup() {
  initializeSerial();
  initializeDisplayBoard();
//  initializeSensors();
}

void loop() {
  float a = thermometer_a.getTemperature();
  float b = thermometer_b.getTemperature();

  if (a > -100.0) {
    temp_a = a;
  } else {
    Serial.println("A failed");
  }
  if (b > -100.0) {
    temp_b = b;
  } else {
    Serial.println("b failed");
  }
  
  float diff_a = temp_a - ref_temp_a;
  float diff_b = temp_b - ref_temp_b;

  Serial.println("");
  Serial.print("a:");
  Serial.print(temp_a);
  Serial.print(":");
  Serial.print(diff_a);
  Serial.print(":b:");
  Serial.print(temp_b);
  Serial.print(":");
  Serial.println(diff_b);
  
  updateDisplay(temp_a, temp_b,
    diff_a < (-1 * tolerance_a),
    diff_a > tolerance_a,
    diff_b < (-1 * tolerance_b),
    diff_b > tolerance_b);
  
 // printButtons();
  delay(1000);
}

#if 0
/*
void initializeSensors() {
    // Start up the library
  sensors.begin();
  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(thermometer_a, 10);
//  sensors.setResolution(outsideThermometer, 10);
//  sensors.setResolution(dogHouseThermometer, 10);
}
*/




void readTemperature(DeviceAddress device, char channel, float& temp_x) {
  float tempC = sensors.getTempC(device);
  if (tempC <= -127.00) {
    Serial.print("Error getting temperature");
  } else {
    float tempF = DallasTemperature::toFahrenheit(tempC);
    Serial.print(channel);
    Serial.print(":");
    Serial.print(tempC);
    Serial.print(":");
    Serial.println(tempF);
    temp_x = tempF;
  }
}

void countUp(float& temp_x) {
  temp_x += 1.0;
}

void countDown(float& temp_x) {
  temp_x -= 1.0;
}

void loop() {

  if (state != prevstate) {
     Serial.print("State = ");
     Serial.println(state);
     prevstate = state;
  }
  
  switch (state) {
          
    case STATE_READ: {
      
        printButtons();
        
        boolean count_up = displayBoard.isButtonPushed(BUTTON_UP);
  
        if (displayBoard.isSwitchLeft()) {
          (count_up ? countUp : countDown)(ref_temp_a);
          updateDisplay(ref_temp_a, CHANNEL_DASHES, true, true, false, false);
        }
        if (displayBoard.isSwitchRight()) {
          (count_up ? countUp : countDown)(ref_temp_b);
          updateDisplay(CHANNEL_DASHES, ref_temp_b, false, false, true, true);
        }
        if (displayBoard.isSwitchCenter()) {
          readTemperatures();
          updateDisplay(temp_a, temp_b,
             temp_a > ref_temp_a, temp_b > ref_temp_b,
             temp_a < ref_temp_a, temp_b < ref_temp_b);
        }
        state = STATE_WAIT_BUTTON_UP;
      }
      break;
      
    case STATE_WAIT_BUTTON_DOWN:
      if (displayBoard.isButtonPushed()) {
        Serial.print("button-down: ");
        Serial.println(WHICH_BUTTON_PUSHED);
      
        state = STATE_READ;
      }
      break;

      
    case STATE_WAIT_BUTTON_UP:
      if (!displayBoard.isButtonPushed()) {
        Serial.println("button-up");
        holdcount = 2000 / DELAYTIME;
        holdperiod = HOLDPERIOD0;
        state = STATE_WAIT_BUTTON_DOWN;
      }
      // Timeout of holdcount while waiting causes countUp/Down
      if (holdcount-- <= 0) {
        holdcount = holdperiod / DELAYTIME; // 3 counts/sec is fast count
        holdperiod = (holdperiod * 95) / 100;
        if (holdperiod = 0) { holdperiod = 1; }
        state = STATE_READ;
      }
      break;
      
    default:
      Serial.print("Illegal state ");
      Serial.println(state);
      delay(10000);
      state = STATE_WAIT_BUTTON_DOWN;
      break;

  }
  delay(DELAYTIME);
}

#endif

