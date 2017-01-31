#ifndef TemperatureStore_h
#define TemperatureStore_h

#include <Arduino.h>

class TemperatureStore {
public:
  TemperatureStore(uint32_t size, uint32_t sampleSize);
  ~TemperatureStore();
  void add(float temperature);
  float getMedian();
  float getMedianLatest(uint32_t size);
  float getMedianOldest(uint32_t size);
  float getTendence();
  bool isFull();
private:
  float * _store;
  uint32_t _size;
  uint32_t _sampleSize;
  uint32_t _currentOffset;
  uint32_t _oldestOffset;
  uint32_t _filledSize;

  float* _getSample(float *result, int offset, int size);
  float _getMedian(float *temperaturesSample, int size);
};

#endif
