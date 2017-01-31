#ifndef ThermistorReader_h
#define ThermistorReader_h

#include <Arduino.h>

class ThermistorReader {
  public:
    ThermistorReader(const int sensorPIN, const float resistorValue, const float voltage, const uint32_t sampleSize, const float A_1, const float B_1, const float C_1, const float D_1);
    float getTemperatureKelvin();
    float getTemperatureCelsius();

  private:
    int _sensorPIN;
    float _resistorValue;
    float _voltage;
    uint32_t _sampleSize;
    float _A_1;
    float _B_1;
    float _C_1;
    float _D_1;

    float _steinhartHart(float thermistorResistanceValue, float resistance);
    float _getResistance(int thermistorValue);
    int _getThermistorRawValue();

};

#endif
