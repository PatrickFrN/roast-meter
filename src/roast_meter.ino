// VERSION 1.0.0-rc.3
#include <Wire.h>
#include "MAX30105.h"
#include <SFE_MicroOLED.h>

#define PIN_RESET 9
#define DC_JUMPER 1

MAX30105 particleSensor;
MicroOLED oled(PIN_RESET, DC_JUMPER);

long samplesTaken = 0;  //Counter for calculating the Hz or read rate
long unblockedValue;    //Average IR at power up
long startTime;         //Used to calculate measurement rate

String multiplyChar(char c, int n) {
  String result = "";
  for (int i = 0; i < n; i++) {
    result += c;
  }
  return result;
}

void displayMeasurement(int rLevel) {
  oled.clear(PAGE);
  oled.setCursor(0, 0);

  int calibratedReading = f(rLevel);
  int centerPadding = 4 - String(calibratedReading).length();
  String paddingText = multiplyChar(' ', centerPadding);

  if (rLevel == 0) {
    oled.setFontType(1);
    oled.print("Please load   sample!");
    oled.display();
    return;
  }

  oled.setFontType(3);
  oled.print(paddingText);
  oled.print(calibratedReading);

  Serial.println("real:" + String(rLevel));
  Serial.println("agtron:" + String(calibratedReading));
  Serial.println("===========================");

  oled.display();
}

int f(int x) {
  int intersectionPoint = 117;
  float deviation = 0.165;

  return x - (intersectionPoint - x) * deviation;
}

byte intToHex(int num) {
  // Convert the integer to hexadecimal string
  String hexString = String(num, HEX);

  // Convert the hexadecimal string to byte
  byte hexByte = 0;
  for (int i = 0; i < hexString.length(); i++) {
    char c = hexString.charAt(i);

    if (isDigit(c)) {
      hexByte = (hexByte << 4) + (c - '0');
    } else {
      c = toUpperCase(c);
      hexByte = (hexByte << 4) + (c - 'A' + 10);
    }
  }

  return hexByte;
}

void setup() {
  Serial.begin(9600);

  Wire.begin();
  oled.begin();      // Initialize the OLED
  oled.clear(ALL);   // Clear the display's internal memory
  oled.clear(PAGE);  // Clear the buffer.

  delay(100);  // Delay 100 ms
  oled.setFontType(3);

  // Initialize sensor
  if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false)  //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1)
      ;
  }

  // Variable used to calibrate your sensor, change the value below +/- 1 till you get an unbleached V60 paper to read between 180-190 values on the screen.
  byte ledBrightness = intToHex(37);  //Options: 0=Off to 255=50mA

  byte sampleAverage = 32;  //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2;         //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  int sampleRate = 3200;    //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411;     //Options: 69, 118, 215, 411
  int adcRange = 4096;      //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);  //Configure sensor with these settings

  //particleSensor.setPulseAmplitudeRed(0);    //Turn off Red LED
  particleSensor.setPulseAmplitudeGreen(0);  //Turn off Green LED


  Serial.print(unblockedValue);
  startTime = millis();

  // Update to ignore readings under 30.000
  unblockedValue = 30000;
}

void loop() {
  samplesTaken++;
  long currentDelta = particleSensor.getIR() - unblockedValue;

  if (currentDelta > (long)100) {
    int rLevel = int(particleSensor.getIR()) / 1000;
    displayMeasurement(rLevel);
  } else {
    displayMeasurement(0);
  }
  delay(100);
}
