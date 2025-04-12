#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include <TimerOne.h>
#include <PID_v1.h>
#include <Servo.h>

// Motor control pins
#define MOTOR_PIN_1 5    // PWM pin for motor control
#define MOTOR_PIN_2 6    // PWM pin for motor control
#define ENABLE_PIN 9     // Enable pin for the motor driver
#define ENCODER_PIN_A 2  // Encoder input A
#define ENCODER_PIN_B 3  // Encoder input B

// Motor parameters
#define MIN_PULSE_FREQUENCY 60   // Minimum pulse rate (BPM)
#define MAX_PULSE_FREQUENCY 120  // Maximum pulse rate (BPM)
#define MIN_FLOW_RATE 0.0       // Minimum flow rate (L/min)
#define MAX_FLOW_RATE 6.0       // Maximum flow rate (L/min)

// PID Control parameters
#define KP 2.0  // Proportional gain
#define KI 5.0  // Integral gain
#define KD 1.0  // Derivative gain

// Function declarations
void initializeMotor();
void setMotorSpeed(int speed);  // Speed range: -255 to 255
void setFlowRate(float rate);   // Rate in L/min
void setPulseFrequency(int bpm);
float getCurrentFlowRate();
int getCurrentPulseRate();
void emergencyStop();

#endif // MOTOR_CONTROL_H 