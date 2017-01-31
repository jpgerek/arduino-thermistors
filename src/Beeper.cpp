#include "Beeper.h"


Beeper::Beeper(int buzzerPin, unsigned int minimumTimeWithoutChangesToBeep) {
  this->_buzzerPin = buzzerPin;
  this->_minimumTimeWithoutChangesToBeep = minimumTimeWithoutChangesToBeep;
  this->_lastBeepingTendence = TemperatureDiffTendence::NONE;
}

void Beeper::beep() {
  for (uint8_t x = 0; x < 3; x++) {
    for (uint8_t y = 0; y < 3; y++) {
      tone(this->_buzzerPin, 9999, 100);
      delay(110);
    }
    delay(500);
  }
  noTone(this->_buzzerPin);
}

void Beeper::_beepIfCountdownIsOver(unsigned long currentTimestamp) {
  if (this->_beepTimestamp > 0 && (currentTimestamp - this->_beepTimestamp) > this->_minimumTimeWithoutChangesToBeep) {
    this->_beepTimestamp = 0;
    this->_lastBeepingTendence = this->_nextBeepingTendence;
    this->beep();
  }
}

void Beeper::update(unsigned long currentTimestamp, TemperatureDiffTendence currentInOutTemperatureDiffTendence, TemperatureDiffTendence outsideTemperatureTendence) {
  if (currentInOutTemperatureDiffTendence != this->_lastTemperatureDiffTendence) {
    if (this->_lastTemperatureDiffTendence != TemperatureDiffTendence::NONE
      && currentInOutTemperatureDiffTendence == TemperatureDiffTendence::EQUAL
      && this->_lastTemperatureDiffTendence == outsideTemperatureTendence
      && outsideTemperatureTendence != TemperatureDiffTendence::EQUAL) {
        // Set countdown timestamp
        if (this->_lastBeepingTendence != currentInOutTemperatureDiffTendence) {
          this->_beepTimestamp = currentTimestamp;
          this->_nextBeepingTendence = currentInOutTemperatureDiffTendence;
        }
    } else {
      // Reset countdown timestamp
      this->_beepTimestamp = 0;
    }
    this->_lastTemperatureDiffTendence = currentInOutTemperatureDiffTendence;
  }
  this->_beepIfCountdownIsOver(currentTimestamp);
}

TemperatureDiffTendence Beeper::getLastTemperatureDiffTendence() {
  return this->_lastTemperatureDiffTendence;
}

unsigned long Beeper::getBeepTimestamp() {
  return this->_beepTimestamp;
}
