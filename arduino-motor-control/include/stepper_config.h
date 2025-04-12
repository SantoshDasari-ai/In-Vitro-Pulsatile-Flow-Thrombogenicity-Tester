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
#define STEP_PIN 5                         // Step pulse pin
#define DIR_PIN 2                          // Direction pin
#define ENABLE_PIN 8                       // Enable pin

// Direction definitions
#define CLOCKWISE 1
#define COUNTERCLOCKWISE 0

// Timing parameters
#define MIN_PULSE_WIDTH_US 2.5             // Minimum pulse width for DM860I (microseconds)
#define MIN_COMMAND_DELAY_US 2.5           // Minimum delay between commands (microseconds)

// Safety limits
#define MAX_TEMPERATURE_C 80               // Maximum motor temperature (Celsius)
#define CURRENT_LIMIT (MOTOR_CURRENT * 0.9) // 90% of rated current for safety

// Cardiac cycle parameters
#define HEART_RATE_BPM 60                  // Heart rate in beats per minute
#define CARDIAC_CYCLE_MS (60000 / HEART_RATE_BPM) // Full cardiac cycle time in milliseconds
#define SYSTOLE_PERCENTAGE 33              // Percentage of cardiac cycle spent in systole (contraction)
#define DIASTOLE_PERCENTAGE (100 - SYSTOLE_PERCENTAGE) // Percentage spent in diastole (relaxation)
#define SYSTOLE_MS ((CARDIAC_CYCLE_MS * SYSTOLE_PERCENTAGE) / 100) // Systole duration in ms
#define DIASTOLE_MS ((CARDIAC_CYCLE_MS * DIASTOLE_PERCENTAGE) / 100) // Diastole duration in ms

// Flow parameters
#define STROKE_VOLUME_ML 70                // Target stroke volume in milliliters per beat
#define TUBE_INNER_DIAMETER_MM 25.4        // Inner diameter of the tube in mm
#define LOOP_CENTER_DIAMETER_INCHES 12     // Overall diameter of the loop in inches
#define LOOP_CENTER_DIAMETER_MM (LOOP_CENTER_DIAMETER_INCHES * 25.4) // Loop diameter in mm

// Angular displacement required for one stroke volume
// This is a starting point and may need experimental calibration
#define LOOP_CIRCUMFERENCE_MM (LOOP_CENTER_DIAMETER_MM * PI) // Circumference of the loop
#define TUBE_CROSS_SECTION_AREA_MM2 (PI * (TUBE_INNER_DIAMETER_MM/2) * (TUBE_INNER_DIAMETER_MM/2)) // Cross-sectional area of tube
#define DISPLACEMENT_PER_STROKE_MM (STROKE_VOLUME_ML / TUBE_CROSS_SECTION_AREA_MM2 * 1000) // Linear displacement for stroke volume
#define ANGULAR_DISPLACEMENT_DEG ((DISPLACEMENT_PER_STROKE_MM / LOOP_CIRCUMFERENCE_MM) * 360) // Angular displacement in degrees
#define STEPS_PER_STROKE ((ANGULAR_DISPLACEMENT_DEG / 360.0) * STEPS_PER_REVOLUTION) // Steps needed for one stroke

// Acceleration profiles
#define SYSTOLE_MAX_SPEED_MODIFIER 1.5     // Systole speed multiplier relative to base speed
#define DIASTOLE_MAX_SPEED_MODIFIER 0.8    // Diastole speed multiplier relative to base speed
#define SYSTOLE_ACCEL_MODIFIER 2.5         // Higher acceleration for systole
#define DIASTOLE_ACCEL_MODIFIER 0.7        // Lower acceleration for diastole

#endif // STEPPER_CONFIG_H 
