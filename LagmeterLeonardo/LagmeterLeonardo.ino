// Gery Casiez
// October 2016
// Works using Arduino IDE 1.8.1 with Ubuntu 16.16, OSX Sierra and Windows 10
// Requires HID-Project: https://github.com/NicoHood/HID/

#include "HID-Project.h"

// Buffer to hold RawHID data.
// If host tries to send more data than this,
// it will respond with an error.
// If the data is not read until the host sends the next data
// it will also respond with an error and the data will be lost.
uint8_t receiveBufOutRep[64]; 

boolean debug = false;

int piezoPin = 0;
int photodiodePin = 2;

volatile unsigned long PiezoLastInterrupted;
volatile unsigned long DiodeLastInterrupted;

void setup() {
  if (debug) {
    Serial.begin(115200);
    while(!Serial);
    Serial.println("Serial console enabled");
  }

  attachInterrupt(digitalPinToInterrupt(piezoPin), piezoInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(photodiodePin), photoDiodeInterrupt, RISING);

  // Set the RawHID OUT report array.
  RawHID.begin(receiveBufOutRep, sizeof(receiveBufOutRep));
  for (int i = 0; i < sizeof(receiveBufOutRep); ++i) {
    receiveBufOutRep[i] = 0;
  }
}

void piezoInterrupt() {
  if (micros() - PiezoLastInterrupted > 200000) { // 200 ms
    sendData('P');
    PiezoLastInterrupted = micros();
  }
}

void photoDiodeInterrupt() {
  if (micros() - DiodeLastInterrupted > 200000) { // 200 ms
    sendData('D');
    DiodeLastInterrupted = micros();
  }
}

void sendData(unsigned char source) {
  unsigned long ts = micros();
  receiveBufOutRep[2] = source;
  receiveBufOutRep[3] = (ts & 0xFF000000) >> 24;
  receiveBufOutRep[4] = (ts & 0xFF0000) >> 16;
  receiveBufOutRep[5] = (ts & 0xFF00) >> 8;
  receiveBufOutRep[6] = ts & 0xFF;
  
  int n = RawHID.write(receiveBufOutRep, sizeof(receiveBufOutRep));
  if (debug) {
    Serial.print("Sent ");
    Serial.print(n);
    Serial.println(" bytes");
    Serial.println(ts);
  }  
}

void loop() {
  /*delay(1000);
  sendData('P');
  delay(1000);
  sendData('D');*/
}
