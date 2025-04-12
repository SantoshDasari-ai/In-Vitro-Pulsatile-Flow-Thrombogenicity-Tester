#include <Arduino.h>
#include <AccelStepper.h>
#include "stepper_config.h"

// Create AccelStepper object for the DM860I driver
// The constructor parameters are:
// 1. Type of interface: DRIVER means a Step and Direction driver
// 2. Step pin number
// 3. Direction pin number
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// Function declarations
void setupStepper();
void emergencyStop();
void printStatus();

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("Initializing Stepper Motor Control...");

  // Configure the enable pin
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW);  // Enable the driver (active LOW)

  // Initialize the stepper motor
  setupStepper();

  Serial.println("Stepper Motor Control Ready!");
  printStatus();
}

void loop() {
  // Check if there's any serial command
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    switch (cmd) {
      case '1': // Start continuous rotation
        stepper.setSpeed(MAX_SPEED_STEPS_PER_SEC);
        stepper.runSpeed();
        Serial.println("Starting continuous rotation");
        break;
      
      case '0': // Stop
        stepper.stop();
        stepper.runToPosition();
        Serial.println("Stopping motor");
        break;
      
      case 'e': // Emergency stop
        emergencyStop();
        break;
      
      case 's': // Print status
        printStatus();
        break;
      
      case 'r': // Reverse direction
        stepper.setSpeed(-stepper.speed());
        Serial.println("Reversing direction");
        break;
    }
  }

  // Run the stepper motor
  stepper.run();
}

void setupStepper() {
  // Configure the stepper motor settings
  stepper.setMaxSpeed(MAX_SPEED_STEPS_PER_SEC);
  stepper.setAcceleration(ACCELERATION_STEPS_PER_SEC_2);
  stepper.setEnablePin(ENABLE_PIN);
  stepper.setPinsInverted(false, false, true); // directionInvert, stepInvert, enableInvert
  stepper.enableOutputs();
}

void emergencyStop() {
  stepper.stop();                    // Stop movement
  stepper.setCurrentPosition(0);     // Reset position
  stepper.disableOutputs();          // Disable motor
  Serial.println("EMERGENCY STOP!");
}

void printStatus() {
  Serial.println("\n--- Stepper Motor Status ---");
  Serial.print("Current Position: ");
  Serial.println(stepper.currentPosition());
  Serial.print("Current Speed: ");
  Serial.println(stepper.speed());
  Serial.print("Target Position: ");
  Serial.println(stepper.targetPosition());
  Serial.print("Distance To Go: ");
  Serial.println(stepper.distanceToGo());
  Serial.println("-------------------------\n");
} 