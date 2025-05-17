#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

// Function declarations
void setup();
void loop();
void enableMotor(bool enable);
void setDirection(bool clockwise);
bool handleMotorStep(bool clockwise, int stepDelay);
void calculateDelays(float accel, int highSpeed);
void initializeSystoleState();

#endif // MAIN_H 