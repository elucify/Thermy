#include <TempController.h>

// We always have to include the library
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

#include <OneWire.h>
#include <DallasTemperature.h>

// Temperature bus is Arduino digital pin 6
#define ONE_WIRE_BUS 6

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Assign the addresses of your 1-Wire temp sensors.
// See the tutorial on how to obtain these addresses:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-address-finder.html

DeviceAddress XinsideThermometer = { 0x28, 0x86, 0x86, 0xC9, 0x02, 0x00, 0x00, 0xF4 };
DeviceAddress thermometer_a = { 0x28, 0xFC, 0x74, 0xC9, 0x02, 0x00, 0x00, 0x86 };
DeviceAddress thermometer_b = { 0x28, 0x6B, 0xDF, 0xDF, 0x02, 0x00, 0x00, 0xC0 };
DeviceAddress dogHouseThermometer = { 0x28, 0x59, 0xBE, 0xDF, 0x02, 0x00, 0x00, 0x9F };

// Number of 
#define HOLDPERIOD0 333

/* we always wait a bit between updates of the display */
int count = 0;
int holdcount = 0;
int holdperiod = HOLDPERIOD0;
byte state, prevstate;
float temp_a, temp_b;
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
   boolean hot_a = false,
   boolean hot_b = false,
   boolean cold_a = false,
   boolean cold_b = false)
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
    ref_temp_a = ref_temp_b = 0.0;
    updateDisplay(temp_a, temp_b, false, false, false, false);    
}

void initializeSensors() {
    // Start up the library
  sensors.begin();
  // set the resolution to 10 bit (good enough?)
  sensors.setResolution(thermometer_a, 10);
//  sensors.setResolution(outsideThermometer, 10);
//  sensors.setResolution(dogHouseThermometer, 10);
}

void setup() {
  initializeSerial();
  initializeDisplayBoard();
  initializeSensors();
}

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

void readTemperatures() {
  sensors.requestTemperatures();
  readTemperature(thermometer_a, 'A', temp_a);
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


