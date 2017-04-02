#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "properties.h"

#define STORAGE_MAX_SIZE 64
#define BASE 100
#define SIGNATURE_SIZE 4
#define HEADER_OFFSET (BASE + SIGNATURE_SIZE + sizeof(unsigned long))
#define ADDR(x) HEADER_OFFSET + sizeof(long)*(x)

struct Persistence{

  int size;
  char signature[SIGNATURE_SIZE];

  Persistence(String s, long *props_runtime, int props_size);
  
  void storeProperties(long *props);
  void storeValue(long *prop);
  void storeValue(uint8_t i, long val);
  void loadValue(long *prop);
  void loadValue(uint8_t i);
  void loadProperties(long *prop);

    void checkFactoryReset(long *props_runtime);
};


#endif
