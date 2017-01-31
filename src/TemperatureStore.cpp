#include "TemperatureStore.h"


// callback function for doing comparisons on floats
int _compareFloats (const void * arg1, const void * arg2) {
  float * a = (float *) arg1;
  float * b = (float *) arg2;

  if (*a < *b)
    return -1;

  if (*a > *b)
    return 1;

  return 0;
}

TemperatureStore::TemperatureStore(uint32_t storeSize, uint32_t sampleSize) {
  this->_store = (float *) malloc(sizeof(float)*storeSize);
  this->_size = storeSize;
  this->_sampleSize = sampleSize;
  this->_currentOffset = 0;
  this->_oldestOffset = 0;
  this->_filledSize = 0;
}

TemperatureStore::~TemperatureStore() {
  free(this->_store);
}

void TemperatureStore::add(float temperature) {
  this->_currentOffset = (this->_currentOffset+1) % this->_size;
  this->_store[this->_currentOffset] = temperature;
  // Moving the offset considering it has to go to the beggining
  // when it hits the end of the array.
  if (this->isFull()) {
    // If the store is full the oldest sample record is the next one in
    // the array.
    this->_oldestOffset = (this->_currentOffset+1) % this->_size;
  } else {
    this->_filledSize++;
  }
}

// XXX test properly
float TemperatureStore::getMedian() {
  float *storeCopy = (float*)malloc(sizeof(float) * this->_filledSize);
  for (uint32_t x = 0; x < this->_filledSize; x++) {
    storeCopy[x] = this->_store[x];
  }
  float result = this->_getMedian(storeCopy, this->_filledSize);
  free(storeCopy);
  return result;
}

float TemperatureStore::_getMedian(float *temperaturesSample, int size) {
  qsort(temperaturesSample, size, sizeof(float), _compareFloats);
  return temperaturesSample[size/2];
}

float TemperatureStore::getTendence() {
  // If there is not enough data to perform a median just return the latest and the oldest temperature stored difference.
  if (!this->isFull() && (this->_currentOffset - this->_oldestOffset) < this->_sampleSize) {
    return 0.0;
  }
  float *recentTemperaturesSample = (float*)malloc(sizeof(float) * this->_sampleSize);
  this->_getSample(recentTemperaturesSample, this->_currentOffset-this->_sampleSize, this->_sampleSize);
  float *oldestTemperaturesSample = (float*) malloc(sizeof(float) * this->_sampleSize);
  this->_getSample(oldestTemperaturesSample, this->_oldestOffset, this->_sampleSize);
  float tendence = this->_getMedian(recentTemperaturesSample, this->_sampleSize) - this->_getMedian(oldestTemperaturesSample, this->_sampleSize);
  free(oldestTemperaturesSample);
  free(recentTemperaturesSample);
  return tendence;
}

bool TemperatureStore::isFull() {
  return this->_filledSize == this->_size;
}

float* TemperatureStore::_getSample(float *result, int offset, int sampleSize) {
    for (int x = 0; x < sampleSize; x++) {
      result[x] = this->_store[(offset+x) % this->_filledSize];
    }
    return result;
}
