#include <iostream>
#include "arraymem.h"


using namespace std;
int main() {

// declare array of 5 ints
int test[5];
int *testptr = test;
// assign memory address of array to ptr
int size_test = 5;
arrayManipulate(testptr, size_test);
for (int i = 0; i < size_test; i++) { // this part is technically illegal in arduino, disregard.
  cout << test[i];
        
  }
      



	return 0;
}
  





