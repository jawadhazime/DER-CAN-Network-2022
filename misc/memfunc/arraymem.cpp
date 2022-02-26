#include "arrayMem.h"
#include <iostream>
void arrayManipulate(int *passer, int size) {
  for (int j = 0; j < size; j++) 
    passer[j] = j % 2;
}
