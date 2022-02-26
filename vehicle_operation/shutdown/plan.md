# Things to do in here
- initialize CAN connection
- set Baud rate
## Read Sensor values
### Accelerometer
- read values in X,Y,Z axis
- read XYZ gyro data
- 6 values (int)
_should be able to fit this into one CAN message_

### Shock Pots 
- read values for all 4 wheels & store them 
- get min, max, mean & store them
- send all values
- should take up one message
- 7 values (int)

### Steering angle 
- read values from one pot (int)
- Q: if space in other message, should it be sent inside of theirs? (i.e. shock pots)


## Faults 
- need to read pins
- BSPD HIGH
- BMS fault
- IMD fault
- 3 values (bool!) 


## What is Needed
- pin numbers off schematic
- values for faults (unless they are activated by the digital pins) 




_tldr; need to get values from 17 pins?_
