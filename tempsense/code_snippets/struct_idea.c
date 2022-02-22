/* 
there are 72 temp sensors. Using a multiplexer, we can use 9 x (multiplexer that reads from 8 sensors) 

this is a rough idea of how to store such a thing on the Teensy before shipping out to CAN
*/
struct multi {
	int temp a;
	int temp b;
	int temp c;
	int temp d;
	int temp e;
	int temp f;
	int temp g;
	int temp h;
};

struct multi multi_two;

struct multi multi_three;

struct multi multi_four;

struct multi multi_five;

struct multi multi_six;

struct multi multi_seven;

struct multi multi_eight;

struct multi multi_nine;
