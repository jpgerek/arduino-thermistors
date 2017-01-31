/**

- Temperature difference between the inside and the outside. 2 leds.
- Temperature outside DOWN or UP in the last 60 minutes. 2 leds.
- Temperature inside DOWN or UP in the last 60 minutes. 2 leds.

*/
#include <math.h>
#include <LiquidCrystal.h>

#include "TemperatureStore.h"
#include "ThermistorReader.h"
#include "TemperatureDiffTendence.h"
#include "Beeper.h"

//#define DEBUG 1
#ifdef DEBUG
#define DEBUG_PRINT(str) Serial.print(str);
#else
#define DEBUG_PRINT(str)
#endif

#define LCD_RS_PIN 6
#define LCD_ENABLE_PIN 7
#define LCD_D4_PIN 5
#define LCD_D5_PIN 4
#define LCD_D6_PIN 3
#define LCD_D7_PIN 2

#define LOOP_DELAY 700 // Milliseconds.

#define BUZZER_PIN 0

#define SENSOR_PIN_INSIDE_THERMISTOR A0
#define SENSOR_PIN_OUTSIDE_THERMISTOR A1

#define RGB_RED_LED_PIN 11
#define RGB_GREEN_LED_PIN 10
#define RGB_BLUE_LED_PIN 9


#define THERMISTOR_SAMPLE_SIZE 5
#define R_VALUE 10000 // Resistor value.
#define V_CC 5.0

#define MAX_LED_INTENSITY 255
#define NO_TEMPERATURE_DIFF_RANGE 0.4 // +/- the value
#define TEMPERATURE_DIFF_TO_LED_INTENSITY_RATIO 3

#define A_1 3.354016E-3
#define B_1 2.569850E-4
#define C_1 2.620131E-6
#define D_1 6.383091E-8

#define TEMPERATURE_RECENT_STATS_SIZE 50

// 60 minutes of temperature stats.
#define TEMPERATURE_STATS_SIZE 60
#define TEMPERATURE_STATS_INTERVAL 60 // Seconds.
#define TEMPERATURE_STATS_SAMPLE_SIZE 5

#define MINIMUM_TIME_WITHOUT_CHANGES_TO_BEEP 60000 // Milliseconds



LiquidCrystal* lcd;

TemperatureStore* inRecentTemperatureStore;
TemperatureStore* outRecentTemperatureStore;

TemperatureStore* inTemperatureStore;
TemperatureStore* outTemperatureStore;

ThermistorReader* inThermistor;
ThermistorReader* outThermistor;

unsigned long lastRunTSMillis = 0;
TemperatureDiffTendence lastTemperatureDiffTendence = TemperatureDiffTendence::NONE;

TemperatureStore* temperatureDiffTendenceStore;

Beeper* temperatureChangeBeep;

void displayTemperatureDiff(int rgbRedPin, int rgbGreenPin, int rgbBluePin, float temperatureDiff) {
  int ledIntensity = abs(temperatureDiff) * TEMPERATURE_DIFF_TO_LED_INTENSITY_RATIO;
  switch (getTemperatureDiffTendence(temperatureDiff)) {
    case UP:
      analogWrite(rgbRedPin, ledIntensity);
      analogWrite(rgbGreenPin, 0);
      analogWrite(rgbBluePin, 0);
    break;
    case DOWN:
    analogWrite(rgbRedPin, 0);
    analogWrite(rgbGreenPin, 0);
    analogWrite(rgbBluePin, ledIntensity);
    break;
    default:
    analogWrite(rgbRedPin, 0);
    analogWrite(rgbGreenPin, 1);
    analogWrite(rgbBluePin, 0);
  }
}

enum TemperatureDiffTendence getTemperatureDiffTendence(float temperatureDiff) {
  if (abs(temperatureDiff) < NO_TEMPERATURE_DIFF_RANGE) {
    return TemperatureDiffTendence::EQUAL;
  }
  if (temperatureDiff > 0.0) {
    return TemperatureDiffTendence::UP;
  }
  return TemperatureDiffTendence::DOWN;
}

void displayInfoInLCD() {


}

void setup() {
  pinMode(RGB_RED_LED_PIN, OUTPUT);
  pinMode(RGB_GREEN_LED_PIN, OUTPUT);
  pinMode(RGB_BLUE_LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.begin(9600);

  lcd = new LiquidCrystal(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);
  lcd->begin(20, 4);
  lcd->noAutoscroll();
  lcd->noCursor();
  lcd->setCursor(0, 0);
  lcd->print("Starting...");

  inRecentTemperatureStore = new TemperatureStore(TEMPERATURE_RECENT_STATS_SIZE, TEMPERATURE_STATS_SAMPLE_SIZE);
  outRecentTemperatureStore = new TemperatureStore(TEMPERATURE_RECENT_STATS_SIZE, TEMPERATURE_STATS_SAMPLE_SIZE);

  inTemperatureStore = new TemperatureStore(TEMPERATURE_STATS_SIZE, TEMPERATURE_STATS_SAMPLE_SIZE);
  outTemperatureStore = new TemperatureStore(TEMPERATURE_STATS_SIZE, TEMPERATURE_STATS_SAMPLE_SIZE);

  inThermistor = new ThermistorReader(SENSOR_PIN_INSIDE_THERMISTOR, R_VALUE, V_CC, THERMISTOR_SAMPLE_SIZE, A_1, B_1, C_1, D_1);
  outThermistor = new ThermistorReader(SENSOR_PIN_OUTSIDE_THERMISTOR, R_VALUE, V_CC, THERMISTOR_SAMPLE_SIZE, A_1, B_1, C_1, D_1);

  temperatureDiffTendenceStore = new TemperatureStore(TEMPERATURE_RECENT_STATS_SIZE, TEMPERATURE_STATS_SAMPLE_SIZE);

  temperatureChangeBeep = new Beeper(BUZZER_PIN, MINIMUM_TIME_WITHOUT_CHANGES_TO_BEEP);
  temperatureChangeBeep->beep();
}

void loop() {
  if ((millis() - lastRunTSMillis) < LOOP_DELAY) {
    return;
  }

  float instantTemperatureInside = inThermistor->getTemperatureCelsius();
  float instantTemperatureOutside = outThermistor->getTemperatureCelsius();

  DEBUG_PRINT(F("instantTemperatureInside = "));
  DEBUG_PRINT(instantTemperatureInside);
  DEBUG_PRINT(F(", instantTemperatureOutside = "));
  DEBUG_PRINT(instantTemperatureOutside);
  DEBUG_PRINT(F(", instant temperature Diff = "));
  DEBUG_PRINT(instantTemperatureOutside-instantTemperatureInside);
  DEBUG_PRINT(F("\n"));

  inRecentTemperatureStore->add(instantTemperatureInside);
  outRecentTemperatureStore->add(instantTemperatureOutside);

  float temperatureInside = inRecentTemperatureStore->getMedian();
  float temperatureOutside = outRecentTemperatureStore->getMedian();
  float temperatureDiff = temperatureOutside - temperatureInside;

  temperatureDiffTendenceStore->add(temperatureDiff);

  TemperatureDiffTendence currentInOutTemperatureDiffTendence = TemperatureDiffTendence::NONE;
  if (inRecentTemperatureStore->isFull()) {
    currentInOutTemperatureDiffTendence = getTemperatureDiffTendence(temperatureDiff);
  }

  DEBUG_PRINT(F("OutsideMedian = "));
  DEBUG_PRINT(temperatureOutside);
  DEBUG_PRINT(F("ºC. "));
  DEBUG_PRINT(F("InsideMedian = "));
  DEBUG_PRINT(temperatureInside);
  DEBUG_PRINT(F("ºC. "));
  DEBUG_PRINT(F("temperatureDiff = "));
  DEBUG_PRINT(temperatureDiff);
  DEBUG_PRINT(F("ºC.            \n"));

  if ((millis()/1000)%TEMPERATURE_STATS_INTERVAL == 0) {
    inTemperatureStore->add(temperatureInside);
    outTemperatureStore->add(temperatureOutside);
  }

  float outsideTemperatureTendence = outTemperatureStore->getTendence();
  float insideTemperatureTendence = inTemperatureStore->getTendence();

  displayTemperatureDiff(RGB_RED_LED_PIN, RGB_GREEN_LED_PIN, RGB_BLUE_LED_PIN, temperatureDiff);

  DEBUG_PRINT(F("Outside temperature tendence: "));
  DEBUG_PRINT(outsideTemperatureTendence);
  DEBUG_PRINT(F("ºC. "));
  DEBUG_PRINT(F("Inside temperature tendence: "));
  DEBUG_PRINT(insideTemperatureTendence);
  DEBUG_PRINT(F("ºC.      \n"));

  DEBUG_PRINT(F("lastTemperatureDiffTendence = "));
  DEBUG_PRINT(temperatureChangeBeep->getLastTemperatureDiffTendence());
  DEBUG_PRINT(F(", currentInOutTemperatureDiffTendence = "));
  DEBUG_PRINT(currentInOutTemperatureDiffTendence);
  DEBUG_PRINT(F("      \n"));

  lastRunTSMillis = millis();

  DEBUG_PRINT(F("beepTimestamp = "));
  DEBUG_PRINT(temperatureChangeBeep->getBeepTimestamp());
  DEBUG_PRINT(F("\n"));
  DEBUG_PRINT(F("lastRunTSMillis = "));
  DEBUG_PRINT(lastRunTSMillis);
  DEBUG_PRINT(F("\n"));

  DEBUG_PRINT(F("\n------------------------\n\n"));

  temperatureChangeBeep->update(millis(), currentInOutTemperatureDiffTendence, getTemperatureDiffTendence(outsideTemperatureTendence));

  lcd->setCursor(0, 0);
  lcd->print("In:" + String(temperatureInside, 2) + " - Out:" + String(temperatureOutside, 2));
  lcd->setCursor(0, 1);
  lcd->print("In/out diff: ");
  if (temperatureDiff > 0.0) {
    lcd->print("+");
  }
  lcd->print(String(temperatureDiff, 2));
  lcd->setCursor(0, 2);
  lcd->print("Tend. out: ");
  if (outsideTemperatureTendence > 0.0) {
    lcd->print("+");
  }
  lcd->print(String(outsideTemperatureTendence, 2));
  lcd->setCursor(0, 3);
  lcd->print("Tend.  in: ");
  if (insideTemperatureTendence > 0.0) {
    lcd->print("+");
  }
  lcd->print(String(insideTemperatureTendence, 2));
}
