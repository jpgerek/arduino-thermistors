#ifndef Beeper_h
#define Beeper_h

#include <Arduino.h>
#include "TemperatureDiffTendence.h"

class Beeper {
public:
  Beeper(int buzzerPin, unsigned int minimumTimeWithoutChangesToBeep);
  ~Beeper();

  void beep();
  void update(unsigned long currentTimestamp, TemperatureDiffTendence currentInOutTemperatureDiffTendence, TemperatureDiffTendence outsideTemperatureTendence);
  TemperatureDiffTendence getLastTemperatureDiffTendence();
  unsigned long getBeepTimestamp();

private:
  int _buzzerPin;
  unsigned long _beepTimestamp;
  unsigned int _minimumTimeWithoutChangesToBeep;
  TemperatureDiffTendence _lastTemperatureDiffTendence;
  TemperatureDiffTendence _lastBeepingTendence;
  TemperatureDiffTendence _nextBeepingTendence;

  void _beepIfCountdownIsOver(unsigned long currentTimestamp);
};

#endif
