#include <Adafruit_Arcada.h>
#include <Adafruit_SPIFlash.h>
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

// GUItool: begin automatically generated code
AudioSynthSimpleDrum     drum1;          //xy=111.42855834960938,587.0000591278076
AudioSynthWaveform       waveform1;      //xy=156.7142791748047,101.4285774230957
AudioSynthWaveform       waveform2;      //xy=157.14286041259766,179.57142448425293
AudioSynthNoisePink      pink1;          //xy=166.42855834960938,636.0000591278076
AudioSynthWaveform       waveform3;      //xy=176.42855834960938,472.0000591278076
AudioSynthWaveformSineHires sine_hires1;    //xy=204.42855834960938,725.0000591278076
AudioSynthWaveformDc     dc1;            //xy=238.42855834960938,819.0000591278076
AudioSynthWaveformSineModulated sine_fm1;       //xy=348.4285583496094,663.0000591278076
AudioSynthKarplusStrong  string1;        //xy=395.4285583496094,599.0000591278076
AudioEffectMultiply      multiply1;      //xy=442.2857131958008,143.85715007781982
AudioSynthToneSweep      tonesweep1;     //xy=446.4285583496094,786.0000591278076
AudioSynthWaveformSine   sine1;          //xy=449.9999809265137,296.2857208251953
AudioSynthWaveformPWM    pwm1;           //xy=573.4285583496094,759.0000591278076
AudioFilterStateVariable filter1;        //xy=624.2857142857142,97.14285714285714
AudioMixer4              mixer1;         //xy=805.7142857142857,112.85714285714285
AudioOutputAnalogStereo  dac1;           //xy=1097.2856063842773,92.71429824829102
AudioConnection          patchCord1(waveform1, 0, multiply1, 0);
AudioConnection          patchCord2(waveform2, 0, multiply1, 1);
AudioConnection          patchCord3(multiply1, 0, filter1, 0);
AudioConnection          patchCord4(sine1, 0, filter1, 1);
AudioConnection          patchCord5(filter1, 1, mixer1, 0);
AudioConnection          patchCord6(mixer1, 0, dac1, 1);
AudioConnection          patchCord7(mixer1, 0, dac1, 0);
// GUItool: end automatically generated code

Adafruit_Arcada arcada;
extern Adafruit_SPIFlash Arcada_QSPI_Flash;

uint32_t buttons, last_buttons;

int joystickX, joystickY;
int xoffset, yoffset; // for analog joystick, static offset

uint8_t j = 0;  // neopixel counter for rainbow

Scheduler taskScheduler;
void readControls();
void updateDisplay();

Task inputLoop (50, TASK_FOREVER, &readControls, &taskScheduler, true);
Task displayLoop (5, TASK_FOREVER, &updateDisplay, &taskScheduler, true);

void setup() {
  AudioMemory(120);
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
  if (!Arcada_QSPI_Flash.begin()) {
    Serial.println("Could not find flash on QSPI bus!");
    arcada.display->setTextColor(ARCADA_RED);
    arcada.display->println("QSPI Flash FAIL");
  }

  arcada.accel->setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G!
  arcada.accel->setClick(1, 80);

  xoffset = arcada.readJoystickX();// record static err
  yoffset = arcada.readJoystickY();

  int illum = arcada.readLightSensor();
  _PP("light "); _PL(illum);

  buttons = last_buttons = 0;

  filter1.frequency(880);
  filter1.octaveControl(1);
  filter1.resonance(1);

  waveform1.begin(.50, 440, WAVEFORM_SAWTOOTH);
  waveform2.begin(.50, 441, WAVEFORM_TRIANGLE);
  mixer1.gain(0, .5);

  arcada.display->fillScreen(ARCADA_BLACK);
  arcada.display->setTextColor(ARCADA_BLUE);
  arcada.display->println("Hey Doc!");
  arcada.display->println(" wassup");
  delay(1000);
  arcada.display->fillRect(0, 70, 160, 60, ARCADA_BLACK);
}

void loop() {
  taskScheduler.execute();

  for (int32_t i = 0; i < arcada.pixels.numPixels(); i++) {
    arcada.pixels.setPixelColor(i, Wheel(((i * 256 / arcada.pixels.numPixels()) + j * 5) & 255));
  }
  arcada.pixels.show();

}

void readControls() {
  uint8_t pressed_buttons = arcada.readButtons();
  last_buttons = buttons;
  buttons = pressed_buttons;
  
  joystickX = arcada.readJoystickX() - xoffset;
  joystickY = arcada.readJoystickY() - yoffset;
  if (buttons & ARCADA_BUTTONMASK_A) {
    float pitch1 = map(joystickX, -512, 511, 120, 240);
    float pitch2 = map(joystickY, -512, 511, .80, 5);
    waveform1.frequency(pitch1);
    waveform2.frequency(pitch2);
  }
  else {
    filter1.frequency(map(joystickX, -512, 511, 120, 2000));
  }

  /*

    #define ARCADA_BUTTONMASK_A 0x01
    #define ARCADA_BUTTONMASK_B 0x02
    #define ARCADA_BUTTONMASK_SELECT 0x04
    #define ARCADA_BUTTONMASK_START 0x08
  */

  //  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
  //  int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  //  distanceSensor.stopRanging();

  uint8_t click = arcada.accel->getClick();
  if (click & 0x30) {
    //    if (click & 0x10) arcada.display->print("single");
    //    if (click & 0x20) arcada.display->print("double");
    //    arcada.display->print(" click detected (0x"); arcada.display->print(click, HEX); arcada.display->println("): ");
  }

  sensors_event_t event;
  arcada.accel->getEvent(&event);

  j++; // update neopixels


  //  arcada.display->print("Z:"); arcada.display->print(event.acceleration.z, 1);
  //  Serial.print("Light: "); Serial.println(arcada.readLightSensor());
  //  float vbat = arcada.readBatterySensor();
}

void updateDisplay() {
  arcada.display->fillScreen(ARCADA_BLACK);
  arcada.display->setCursor(0, 0);
  if (buttons & ARCADA_BUTTONMASK_A) {
    arcada.display->drawCircle(145, 100, 10, ARCADA_RED);
  }
  if (buttons & ARCADA_BUTTONMASK_B) {
    arcada.display->drawCircle(120, 100, 10, ARCADA_YELLOW);
  }
  if (buttons & ARCADA_BUTTONMASK_SELECT) {
    arcada.display->drawRoundRect(60, 100, 20, 10, 5, ARCADA_DARKGREY);
  }
  if (buttons & ARCADA_BUTTONMASK_START) {
    arcada.display->drawRoundRect(85, 100, 20, 10, 5, ARCADA_WHITE);
  }

  arcada.display->drawCircle(20, 100, 20, ARCADA_WHITE);
  arcada.display->fillCircle(joystickX, joystickY, 5, ARCADA_WHITE);

}

uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return arcada.pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return arcada.pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return arcada.pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
