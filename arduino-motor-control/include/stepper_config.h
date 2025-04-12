#ifndef STEPPER_CONFIG_H
#define STEPPER_CONFIG_H

// Motor: StepperOnline 34HS38-5204S
#define MOTOR_STEPS 200                     // 1.8 degrees per step
#define MOTOR_CURRENT 5.2                   // Rated current per phase (A)
#define MOTOR_HOLDING_TORQUE 8.5           // Holding torque (N.m)
#define MOTOR_RESISTANCE 0.9               // Phase resistance (Ohm)
#define MOTOR_INDUCTANCE 4.6               // Phase inductance (mH)

// Driver: StepperOnline DM860I
#define DRIVER_VOLTAGE_MIN 24              // Minimum supply voltage (V)
#define DRIVER_VOLTAGE_MAX 80              // Maximum supply voltage (V)
#define DRIVER_CURRENT_MIN 2.4             // Minimum output current (A)
#define DRIVER_CURRENT_MAX 7.2             // Maximum output current (A)
#define DRIVER_MAX_PULSE_FREQ 200000       // Maximum pulse frequency (Hz)

// Control parameters
#define MICROSTEPS 16                      // Microstepping setting
#define STEPS_PER_REVOLUTION (MOTOR_STEPS * MICROSTEPS)
#define MAX_SPEED_RPM 60                   // Maximum speed in RPM
#define MAX_SPEED_STEPS_PER_SEC ((MAX_SPEED_RPM * STEPS_PER_REVOLUTION) / 60)
#define ACCELERATION_STEPS_PER_SEC_2 1000  // Acceleration in steps/sec^2

// Pin definitions
#define STEP_PIN 3                         // Step pulse pin
#define DIR_PIN 2                          // Direction pin
#define ENABLE_PIN 4                       // Enable pin

// Direction definitions
#define CLOCKWISE 1
#define COUNTERCLOCKWISE 0

// Timing parameters
#define MIN_PULSE_WIDTH_US 2.5             // Minimum pulse width for DM860I (microseconds)
#define MIN_COMMAND_DELAY_US 2.5           // Minimum delay between commands (microseconds)

// Safety limits
#define MAX_TEMPERATURE_C 80               // Maximum motor temperature (Celsius)
#define CURRENT_LIMIT (MOTOR_CURRENT * 0.9) // 90% of rated current for safety

#endif // STEPPER_CONFIG_H 