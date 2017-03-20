#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "properties.h"

#define STORAGE_MAX_SIZE 64
#define SIGNATURE_SIZE 4
#define HEADER_OFFSET (SIGNATURE_SIZE + sizeof(unsigned long))
#define ADDR(x) HEADER_OFFSET + sizeof(long)*(x)

struct Persistence{

  int size;
  char signature[SIGNATURE_SIZE];

  Persistence(){}
  Persistence(String s, long* props_runtime, int props_size);
  
  void storeProperties(long* props);
  void storeValue(long* prop);
  void storeValue(int i, long val);
  void loadValue(long* prop);
  void loadValue(int i);
  void loadProperties(long* prop);

private:

  int addrs(int index){
    return HEADER_OFFSET + sizeof(long);
  }

};


#endif
