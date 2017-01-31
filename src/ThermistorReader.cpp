#include "ThermistorReader.h"

#include <math.h>


ThermistorReader::ThermistorReader(int sensorPIN, float resistorValue, float voltage, uint32_t sampleSize, float A_1, float B_1, float C_1, float D_1) {
  this->_sensorPIN = sensorPIN;
  this->_resistorValue = resistorValue;
  this->_voltage = voltage;
  this->_sampleSize = sampleSize;
  this->_A_1 = A_1;
  this->_B_1 = B_1;
  this->_C_1 = C_1;
  this->_D_1 = D_1;
}

float ThermistorReader::getTemperatureKelvin() {
  int thermistorRawValue = this->_getThermistorRawValue();
  float resistance = this->_getResistance(thermistorRawValue);
  return this->_steinhartHart(resistance, this->_resistorValue);
}

float ThermistorReader::getTemperatureCelsius() {
  return getTemperatureKelvin() - 273.15;
}

// Return degress in kelvin
float ThermistorReader::_steinhartHart(float thermistorResistanceValue, float resistance) {
  float E = log(thermistorResistanceValue/resistance);
  return 1.0 / (this->_A_1 + (this->_B_1 * E) + (this->_C_1 * (E*E)) + (this->_D_1 * (E*E*E)));
}

float ThermistorReader::_getResistance(int thermistorValue) {
  float vaults = (float)thermistorValue / 1024 * this->_voltage;
  return (thermistorValue * vaults) / (this->_voltage - vaults);
}

// callback function for doing comparisons on ints
int compareInts (const void * arg1, const void * arg2) {
  int * a = (int *) arg1;
  int * b = (int *) arg2;

  if (*a < *b)
    return -1;

  if (*a > *b)
    return 1;

  return 0;
}

// It get some samples and returns the median of them.
int ThermistorReader::_getThermistorRawValue() {
  int * samples = (int*)malloc(this->_sampleSize*sizeof(int));
  for (uint32_t x = 0; x < this->_sampleSize; x++) {
    samples[x] = analogRead(this->_sensorPIN);
  }
  qsort(samples, sizeof(samples)/sizeof(int), sizeof(int), compareInts);
  int result = samples[this->_sampleSize/2];
  free(samples);
  return result;
}
