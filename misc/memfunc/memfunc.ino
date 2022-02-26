#include "arrayMem.h"

// declare array of 5 ints
int test[5];
int *testptr;
// assign memory address of array to ptr
testptr = test;
int size_test = 5;
void setup() 
  {
      arrayManipulate(*testptr, size_test);
      for (int i = 0; i < size_test; i++) { // this part is technically illegal in arduino, disregard.
        cout << test[i];
        
      }
      
  }


void loop()
{
  
  // call a function
}
