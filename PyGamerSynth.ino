#include <Adafruit_Arcada.h>
#include <Adafruit_SPIFlash.h>
#include "audio.h"
#include <Audio.h>
#include "Adafruit_seesaw.h"
#include <TaskScheduler.h>

// ==== Debug and Test options ==================
#define _DEBUG_
//#define _TEST_

//===== Debugging macros ========================
#ifdef _DEBUG_
#define SerialD Serial
#define _PM(a) SerialD.print(millis()); SerialD.print(": "); SerialD.println(a)
#define _PP(a) SerialD.print(a)
#define _PL(a) SerialD.println(a)
#define _PX(a) SerialD.println(a, HEX)
#else
#define _PM(a)
#define _PP(a)
#define _PL(a)
#define _PX(a)
#endif

AudioSynthWaveform       waveform2;      //xy=165,222
AudioSynthSimpleDrum     drum1;          //xy=235,398
AudioSynthWaveform       waveform1;      //xy=266,71
AudioSynthNoisePink      pink1;          //xy=290,447
AudioSynthWaveform       waveform3;      //xy=300,283
AudioSynthWaveformSineHires sine_hires1;    //xy=328,536
AudioSynthWaveformDc     dc1;            //xy=362,630
AudioSynthWaveformSineModulated sine_fm1;       //xy=472,474
AudioEffectMultiply      multiply1;      //xy=483,162
AudioSynthKarplusStrong  string1;        //xy=519,410
AudioSynthToneSweep      tonesweep1;     //xy=570,597
AudioSynthWaveformPWM    pwm1;           //xy=697,570
AudioSynthWaveformSine   sine1;          //xy=785,363
AudioOutputAnalogStereo  dac1;           //xy=818,178
AudioConnection          patchCord1(waveform2, 0, multiply1, 1);
AudioConnection          patchCord2(waveform1, 0, multiply1, 0);
AudioConnection          patchCord3(multiply1, 0, dac1, 0);
AudioConnection          patchCord4(multiply1, 0, dac1, 1);
// GUItool: end automatically generated code


Adafruit_Arcada arcada;
extern Adafruit_SPIFlash Arcada_QSPI_Flash;

uint32_t buttons, last_buttons;
uint8_t j = 0;  // neopixel counter for rainbow

int xoffset, yoffset; // for analog joystick

Scheduler taskScheduler;
#define PERIOD1 50
void readControls();
Task tBlink1 (PERIOD1, TASK_FOREVER, &readControls, &taskScheduler, true);

//// Check the timer callback, this function is called every millisecond!
//volatile uint16_t milliseconds = 0;
//void timercallback() {
//  analogWrite(LED_BUILTIN, milliseconds);  // pulse the LED
//  if (milliseconds == 0) {
//    milliseconds = 255;
//  } else {
//    milliseconds--;
//  }
//}

void setup() {
  AudioMemory(120);
  Serial.println("Hello! Arcada PyGamer test");
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  arcada.displayBegin();
  arcada.setBacklight(200);
  delay(1);
  arcada.display->setCursor(0, 0);
  arcada.display->setTextWrap(true);

  /********** Check QSPI manually */
  if (!Arcada_QSPI_Flash.begin()){
    Serial.println("Could not find flash on QSPI bus!");
    arcada.display->setTextColor(ARCADA_RED);
    arcada.display->println("QSPI Flash FAIL");
  }
  arcada.accel->setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!
  arcada.accel->setClick(1, 80);

  xoffset = arcada.readJoystickX();// record static err
  yoffset = arcada.readJoystickY();
  
  buttons = last_buttons = 0;

//  arcada.timerCallback(1000, timercallback);

  waveform1.frequency(440);
  waveform1.amplitude(.50);
  waveform2.frequency(441);
  waveform2.amplitude(.50);

  
  arcada.display->setTextColor(ARCADA_BLUE);
  arcada.display->println("Hey Doc!");
  arcada.display->println(" wassup");
}

void loop() {
  taskScheduler.execute();

  for(int32_t i=0; i< arcada.pixels.numPixels(); i++) {
     arcada.pixels.setPixelColor(i, Wheel(((i * 256 / arcada.pixels.numPixels()) + j*5) & 255));
  }
  arcada.pixels.show();

}

void readControls() {
  int x = arcada.readJoystickX() - xoffset;
  int y = arcada.readJoystickY() - yoffset;
  _PP("X: ");_PP(x);_PP(",  Y: "); _PL(y);

  float pitch1 = map(x, -512, 511, 120, 240);
  float pitch2 = map(y, -512, 511, .5, 5);
 
  waveform1.frequency(pitch1);
  waveform2.frequency(pitch2);

  uint8_t pressed_buttons = arcada.readButtons(); 
  last_buttons = buttons;
  buttons = pressed_buttons;
  
//  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
//  int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
//  distanceSensor.stopRanging();
 
 uint8_t click = arcada.accel->getClick();
  if (click & 0x30) {
    if (click & 0x10) arcada.display->print("single");
    if (click & 0x20) arcada.display->print("double");
    arcada.display->print(" click detected (0x"); arcada.display->print(click, HEX); arcada.display->println("): ");
   }

  sensors_event_t event; 
  arcada.accel->getEvent(&event);

//  arcada.display->print("Z:"); arcada.display->print(event.acceleration.z, 1);
//  Serial.print("Light: "); Serial.println(arcada.readLightSensor());
//  float vbat = arcada.readBatterySensor();
   

}


uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return arcada.pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return arcada.pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return arcada.pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
