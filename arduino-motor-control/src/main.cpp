#include <Arduino.h>
#include <AccelStepper.h>
#include "stepper_config.h"

// Create AccelStepper object for the DM860I driver
// The constructor parameters are:
// 1. Type of interface: DRIVER means a Step and Direction driver
// 2. Step pin number
// 3. Direction pin number
#define DIR_PIN 2    // Direction pin
#define STEP_PIN 5   // Step pin
#define ENABLE_PIN 8 // Enable pin

AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// Cardiac cycle state tracking
enum CardiacPhase {
  SYSTOLE,
  DIASTOLE,
  IDLE
};

// Global variables for cardiac cycle control
CardiacPhase currentPhase = IDLE;
unsigned long phaseStartTime = 0;
unsigned long cycleStartTime = 0;
long systoleSteps = 0;
bool cycleInProgress = false;
int heartRateBpm = HEART_RATE_BPM;
float systolePercentage = SYSTOLE_PERCENTAGE;

// Function declarations
void setupStepper();
void emergencyStop();
void printStatus();
void startCardiacCycle();
void updateCardiacCycle();
void configureSystolePhase();
void configureDiastolePhase();
void adjustHeartRate(int newBpm);
void adjustStrokeVolume(float volumePercentage);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("Initializing Cardiac Simulation System...");

  // Configure the enable pin
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW);  // Enable the driver (active LOW)

  // Initialize the stepper motor
  setupStepper();

  // Calculate the number of steps needed for the systole phase
  // This will be equal to the steps for diastole since they should be symmetric
  systoleSteps = STEPS_PER_STROKE / 2;

  Serial.println("Cardiac Simulation System Ready!");
  Serial.println("Commands:");
  Serial.println("  1: Start cardiac cycle simulation");
  Serial.println("  0: Stop simulation");
  Serial.println("  e: Emergency stop");
  Serial.println("  s: Print status");
  Serial.println("  +: Increase heart rate by 5 BPM");
  Serial.println("  -: Decrease heart rate by 5 BPM");
  Serial.println("  v+: Increase stroke volume by 10%");
  Serial.println("  v-: Decrease stroke volume by 10%");
  printStatus();
}

void loop() {
  // Check if there's any serial command
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    if (command == "1") {
      startCardiacCycle();
      Serial.println("Starting cardiac cycle simulation");
    }
    else if (command == "0") {
      cycleInProgress = false;
      stepper.stop();
      currentPhase = IDLE;
      Serial.println("Stopping simulation");
    }
    else if (command == "e") {
      emergencyStop();
    }
    else if (command == "s") {
      printStatus();
    }
    else if (command == "+") {
      adjustHeartRate(heartRateBpm + 5);
    }
    else if (command == "-") {
      adjustHeartRate(heartRateBpm - 5);
    }
    else if (command == "v+") {
      adjustStrokeVolume(1.1); // Increase by 10%
    }
    else if (command == "v-") {
      adjustStrokeVolume(0.9); // Decrease by 10%
    }
  }

  // Update cardiac cycle if in progress
  if (cycleInProgress) {
    updateCardiacCycle();
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

void startCardiacCycle() {
  // Reset and start the cardiac cycle
  cycleInProgress = true;
  currentPhase = SYSTOLE;
  cycleStartTime = millis();
  phaseStartTime = cycleStartTime;
  
  // Configure for systole phase
  configureSystolePhase();
}

void updateCardiacCycle() {
  unsigned long currentTime = millis();
  unsigned long phaseDuration = currentTime - phaseStartTime;
  unsigned long cycleDuration = currentTime - cycleStartTime;
  
  // Calculate adaptive timing based on current heart rate
  unsigned long cardiacCycleMs = 60000 / heartRateBpm;
  unsigned long systoleMs = (cardiacCycleMs * systolePercentage) / 100;
  unsigned long diastoleMs = cardiacCycleMs - systoleMs;
  
  // Check for phase transition or cycle completion
  if (currentPhase == SYSTOLE && phaseDuration >= systoleMs) {
    // Transition from systole to diastole
    currentPhase = DIASTOLE;
    phaseStartTime = currentTime;
    Serial.println("Phase transition: SYSTOLE → DIASTOLE (valve closure)");
    configureDiastolePhase();
  }
  else if (currentPhase == DIASTOLE && phaseDuration >= diastoleMs) {
    // Complete the cycle and start a new one
    Serial.print("Cardiac cycle completed in ");
    Serial.print(cycleDuration);
    Serial.println(" ms");
    
    // Start a new cycle
    currentPhase = SYSTOLE;
    cycleStartTime = currentTime;
    phaseStartTime = currentTime;
    configureSystolePhase();
  }
  
  // Check if we've reached the target position
  if (stepper.distanceToGo() == 0) {
    if (currentPhase == SYSTOLE) {
      // If we reached the systole target position early, wait for the phase transition
      // This could happen if the motor moves faster than expected
      Serial.println("Systole position reached, waiting for phase transition...");
    }
    else if (currentPhase == DIASTOLE) {
      // If we reached the diastole target position early, wait for the phase transition
      Serial.println("Diastole position reached, waiting for phase transition...");
    }
  }
}

void configureSystolePhase() {
  // Configure for rapid, forceful systole movement (clockwise)
  long currentPosition = stepper.currentPosition();
  long targetPosition = currentPosition + systoleSteps;
  
  // Set higher speed and acceleration for systole
  stepper.setMaxSpeed(MAX_SPEED_STEPS_PER_SEC * SYSTOLE_MAX_SPEED_MODIFIER);
  stepper.setAcceleration(ACCELERATION_STEPS_PER_SEC_2 * SYSTOLE_ACCEL_MODIFIER);
  
  // Move to the target position
  stepper.moveTo(targetPosition);
  
  Serial.print("Systole phase started. Moving from ");
  Serial.print(currentPosition);
  Serial.print(" to ");
  Serial.print(targetPosition);
  Serial.println(" steps.");
}

void configureDiastolePhase() {
  // Configure for slower, gentler diastole movement (counterclockwise)
  long currentPosition = stepper.currentPosition();
  long targetPosition = currentPosition - systoleSteps; // Return to starting position
  
  // Set lower speed and acceleration for diastole
  stepper.setMaxSpeed(MAX_SPEED_STEPS_PER_SEC * DIASTOLE_MAX_SPEED_MODIFIER);
  stepper.setAcceleration(ACCELERATION_STEPS_PER_SEC_2 * DIASTOLE_ACCEL_MODIFIER);
  
  // Move to the target position
  stepper.moveTo(targetPosition);
  
  Serial.print("Diastole phase started. Moving from ");
  Serial.print(currentPosition);
  Serial.print(" to ");
  Serial.print(targetPosition);
  Serial.println(" steps.");
}

void adjustHeartRate(int newBpm) {
  // Ensure heart rate is within reasonable bounds
  if (newBpm < 30) newBpm = 30;
  if (newBpm > 120) newBpm = 120;
  
  heartRateBpm = newBpm;
  Serial.print("Heart rate adjusted to ");
  Serial.print(heartRateBpm);
  Serial.println(" BPM");
}

void adjustStrokeVolume(float volumePercentage) {
  // Adjust the systole steps by the given percentage
  systoleSteps = round(systoleSteps * volumePercentage);
  
  // Ensure we don't exceed reasonable values
  long maxSteps = STEPS_PER_REVOLUTION / 2; // Max half revolution
  if (systoleSteps > maxSteps) systoleSteps = maxSteps;
  if (systoleSteps < 10) systoleSteps = 10;
  
  Serial.print("Stroke volume adjusted. Systole steps: ");
  Serial.println(systoleSteps);
}

void emergencyStop() {
  stepper.stop();                    // Stop movement
  stepper.setCurrentPosition(0);     // Reset position
  stepper.disableOutputs();          // Disable motor
  cycleInProgress = false;
  currentPhase = IDLE;
  Serial.println("EMERGENCY STOP!");
}

void printStatus() {
  Serial.println("\n--- Cardiac Simulation Status ---");
  Serial.print("Current Phase: ");
  switch (currentPhase) {
    case SYSTOLE: Serial.println("SYSTOLE"); break;
    case DIASTOLE: Serial.println("DIASTOLE"); break;
    case IDLE: Serial.println("IDLE"); break;
  }
  Serial.print("Heart Rate: ");
  Serial.print(heartRateBpm);
  Serial.println(" BPM");
  Serial.print("Cycle Duration: ");
  Serial.print(60000 / heartRateBpm);
  Serial.println(" ms");
  Serial.print("Systole Percentage: ");
  Serial.print(systolePercentage);
  Serial.println("%");
  Serial.print("Stroke Volume: ~");
  Serial.print(STROKE_VOLUME_ML * (systoleSteps / (STEPS_PER_STROKE / 2.0)));
  Serial.println(" ml (estimated)");
  Serial.print("Steps per Stroke: ");
  Serial.println(systoleSteps * 2);
  Serial.print("Current Position: ");
  Serial.println(stepper.currentPosition());
  Serial.print("Current Speed: ");
  Serial.println(stepper.speed());
  Serial.print("Target Position: ");
  Serial.println(stepper.targetPosition());
  Serial.print("Distance To Go: ");
  Serial.println(stepper.distanceToGo());
  Serial.println("------------------------------\n");
} 
