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

// This Arduino sketch reads DS18B20 "1-Wire" digital
// temperature sensors.
// Tutorial:
// http://www.hacktronics.com/Tutorials/arduino-1-wire-tutorial.html

static TempController *tempController;

void initializeSerial() {
    // Serial
    Serial.begin(9600);
    while (!Serial) {
    }
    Serial.println("Boot");
    tempController = new TempController();
}

void setup() {
  initializeSerial();
}

void loop() {
    tempController->run();
}

