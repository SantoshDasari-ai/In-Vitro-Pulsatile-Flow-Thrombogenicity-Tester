#include "main.h"
#include <math.h>

// Define pin numbers for motor control
constexpr int DIR_PIN = 2;    // Direction pin
constexpr int STEP_PIN = 5;   // Step pin
constexpr int ENABLE_PIN = 8; // Enable pin

// Debug LED pin
constexpr int DEBUG_LED = 13; // Built-in LED

constexpr int STEPS = 600;    // Number of steps per phase (half of 400 steps/rev)
constexpr int HEART_RATE = 60; // Target heart rate in beats per minute

// Overall runtime in seconds
constexpr unsigned long RUNTIME_SECONDS = 2700; // Set runtime here (e.g., 45 minutes)
unsigned long runtimeMillis = RUNTIME_SECONDS * 1000UL; // Convert runtime to milliseconds
unsigned long startMillis;
unsigned long lastStepTime = 0;

// Position tracking relative to cycle start position
volatile long cyclePosition = 0; // Track position relative to start of cycle
long initialPosition = 0;  // Store the initial position as home
constexpr int HALF_REV = 200;   // Half revolution (400/2 steps)
constexpr int QUARTER_REV = 100; // Quarter revolution for each phase (400/4 steps)

// State machine states
enum class State {
  SYSTOLE_ACCEL,  // Always start with systole
  SYSTOLE_DECEL,
  DIASTOLE_ACCEL,
  DIASTOLE_DECEL,
  RETURN_TO_START,
  CYCLE_COMPLETE,
  SHUTDOWN,
  HOLD_POSITION,
  RETURN_TO_MANUAL_POSITION  // State for returning to manually set position
};

// Initialize state to systole
State currentState = State::SYSTOLE_ACCEL;
int currentStep = 0;
int returnStepsRemaining = 0;
bool returnDirection = true;

// Motor control parameters
int delays[STEPS];  // Array to store delay times
const float angle = 1; // Angle of rotation per step
bool motorDirection = true;

// Add new variable to track if we should shutdown after cycle
bool shutdownRequested = false;
bool completeCurrentCycle = false;  // Flag to track if we should complete current cycle

// Add new variable to track manually set position
long manualPosition = 0;

void calculateDelays(float maxSpeed, float minSpeed, bool fullSine = true) {
  // maxSpeed and minSpeed are in microseconds (smaller = faster)
  
  if (fullSine) {
    // Full sinusoidal profile (acceleration and deceleration)
    float speedAmplitude = (minSpeed - maxSpeed) / 2.0f;
    float midSpeed = maxSpeed + speedAmplitude;
    
    for (int i = 0; i < STEPS; i++) {
      // Map to 0 to PI
      float angle = PI * static_cast<float>(i) / static_cast<float>(STEPS - 1);
      float delay = midSpeed - speedAmplitude * sin(angle);
      
      // Ensure delay stays within bounds
      if (delay < maxSpeed) delay = maxSpeed;
      if (delay > minSpeed) delay = minSpeed;
      
      delays[i] = static_cast<int>(delay);
    }
  } else {
    // Half sinusoidal profile (just acceleration or deceleration)
    for (int i = 0; i < STEPS; i++) {
      // Map to 0 to PI/2 (just quarter cycle of sine wave)
      float angle = PI/2 * static_cast<float>(i) / static_cast<float>(STEPS - 1);
      
      // Calculate delay: starts at minSpeed, smoothly reaches maxSpeed
      float ratio = sin(angle);  // 0 to 1
      float delay = minSpeed - (minSpeed - maxSpeed) * ratio;
      
      // Ensure delay stays within bounds
      if (delay < maxSpeed) delay = maxSpeed;
      if (delay > minSpeed) delay = minSpeed;
      
      delays[i] = static_cast<int>(delay);
    }
  }
}

void enableMotor(bool enable) {
  digitalWrite(ENABLE_PIN, enable ? LOW : HIGH);
}

void setDirection(bool clockwise) {
  motorDirection = clockwise;
  digitalWrite(DIR_PIN, clockwise ? HIGH : LOW);  // HIGH for clockwise when looking at shaft
}

bool handleMotorStep(bool clockwise, int stepDelay) {
  unsigned long currentMicros = micros();
  if (currentMicros - lastStepTime >= stepDelay) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(2);
    digitalWrite(STEP_PIN, LOW);
    // Consistent position tracking: increment for counter-clockwise, decrement for clockwise
    cyclePosition += (clockwise ? -1 : 1);
    lastStepTime = currentMicros;
    return true;
  }
  return false;
}

void initializeSystoleState() {
  // Reset all parameters for systole
  currentState = State::SYSTOLE_ACCEL;
  currentStep = 0;
  cyclePosition = 0;
  // Faster movement for systole (contraction) phase
  // Parameters: maxSpeed (µs), minSpeed (µs)
  calculateDelays(1, 300, false);  // False for half-sine (acceleration only)
  setDirection(true);  // clockwise for systole (contraction)
}

void setup() {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(ENABLE_PIN, OUTPUT);
  pinMode(DEBUG_LED, OUTPUT);

  digitalWrite(ENABLE_PIN, LOW); // Enable motor driver
  
  // Store the initial position as home
  initialPosition = cyclePosition;
  
  // Initialize for normal operation
  startMillis = millis();
  initializeSystoleState();
}

// Function to handle manual position commands if needed
void handleSerialCommands() {
  // Empty implementation - manual control removed
}

void loop() {
  unsigned long currentMillis = millis();
  // Check if runtime exceeded, but only set the flag
  if (currentMillis - startMillis >= runtimeMillis && !shutdownRequested) {
    shutdownRequested = true;
    completeCurrentCycle = true;  // Set flag to complete current cycle
  }

  // Handle serial commands
  handleSerialCommands();

  switch (currentState) {
    case State::SYSTOLE_ACCEL:
      if (handleMotorStep(true, delays[currentStep])) {  // true for clockwise (systole/contraction)
        currentStep++;
        if (currentStep >= STEPS/2) {  // Use first half of STEPS for acceleration
          currentStep = 0;
          calculateDelays(1, 300, false);  // Setup for deceleration phase
          currentState = State::SYSTOLE_DECEL;
        }
      }
      break;

    case State::SYSTOLE_DECEL:
      if (handleMotorStep(true, delays[STEPS - currentStep - 1])) {  // Use reversed array for deceleration
        currentStep++;
        if (currentStep >= STEPS/2) {  // Use second half of STEPS for deceleration
          currentStep = 0;
          // Slower movement for diastole (relaxation) phase
          calculateDelays(15, 400, false);  // Setup for diastole acceleration
          setDirection(false);  // counter-clockwise for diastole (relaxation)
          currentState = State::DIASTOLE_ACCEL;
        }
      }
      break;

    case State::DIASTOLE_ACCEL:
      if (handleMotorStep(false, delays[currentStep])) {  // false for counter-clockwise (diastole/relaxation)
        currentStep++;
        if (currentStep >= STEPS/2) {  // Use first half of STEPS for acceleration
          currentStep = 0;
          calculateDelays(15, 400, false);  // Setup for deceleration phase
          currentState = State::DIASTOLE_DECEL;
        }
      }
      break;

    case State::DIASTOLE_DECEL:
      if (handleMotorStep(false, delays[STEPS - currentStep - 1])) {  // Use reversed array for deceleration
        currentStep++;
        if (currentStep >= STEPS/2) {  // Use second half of STEPS for deceleration
          // If shutdown was requested and we're in diastole, go to shutdown
          if (shutdownRequested && completeCurrentCycle) {
            completeCurrentCycle = false;
            currentState = State::SHUTDOWN;
          } else if (cyclePosition != initialPosition) {
            // If not at start, do return
            currentState = State::RETURN_TO_START;
            returnStepsRemaining = abs(cyclePosition - initialPosition);
            returnDirection = cyclePosition < initialPosition;
            setDirection(returnDirection);
          } else {
            // If at start and shutdown requested, go to shutdown
            if (shutdownRequested) {
              currentState = State::SHUTDOWN;
            } else {
              // Otherwise continue with next cycle
              currentState = State::CYCLE_COMPLETE;
              calculateDelays(1, 300, false);  // Setup for next systole acceleration
              setDirection(true);  // clockwise for next systole (contraction)
              currentStep = 0;
            }
          }
        }
      }
      break;

    case State::RETURN_TO_START:
      if (returnStepsRemaining > 0) {
        returnDirection = cyclePosition < initialPosition;  // Recalculate direction
        setDirection(returnDirection);
        if (handleMotorStep(returnDirection, 40)) {  // Extremely fast return
          returnStepsRemaining--;
        }
      } else {
        // Check if we're at initial position
        if (abs(cyclePosition - initialPosition) < 5) {  // Allow small tolerance
          cyclePosition = initialPosition;  // Force to exact initial position
          // If shutdown was requested, go to shutdown
          if (shutdownRequested) {
            currentState = State::SHUTDOWN;
          } else {
            currentState = State::CYCLE_COMPLETE;
            calculateDelays(1, 300, false);  // Reset to systole speed for next cycle
            setDirection(true);  // clockwise for systole
            currentStep = 0;
          }
        } else {
          // If not at initial position, recalculate return
          returnStepsRemaining = abs(cyclePosition - initialPosition);
          returnDirection = cyclePosition < initialPosition;
        }
      }
      break;

    case State::CYCLE_COMPLETE:
      // Only start new cycle if not shutting down
      if (!shutdownRequested) {
        currentState = State::SYSTOLE_ACCEL;
      }
      break;

    case State::SHUTDOWN:
      if (abs(cyclePosition - initialPosition) > 5) {  // Check if we're not at home with tolerance
        // Return to initial position before final shutdown
        returnStepsRemaining = abs(cyclePosition - initialPosition);
        returnDirection = cyclePosition < initialPosition;
        setDirection(returnDirection);
        
        if (handleMotorStep(returnDirection, 250)) {  // Moderate speed return
          returnStepsRemaining--;
        }
      } else {
        cyclePosition = initialPosition;  // Force to exact initial position
        currentState = State::HOLD_POSITION;
        // Keep motor enabled to maintain position at end of runtime
      }
      break;

    case State::HOLD_POSITION:
      // Keep motor enabled and actively holding position
      // No movement needed if we're at the correct position
      break;

    case State::RETURN_TO_MANUAL_POSITION:
      if (cyclePosition != manualPosition) {
        bool dir = cyclePosition < manualPosition;
        setDirection(dir);
        if (handleMotorStep(dir, 40)) {  // Fast return to position
          if (dir) {
            cyclePosition++;
          } else {
            cyclePosition--;
          }
        }
      } else {
        currentState = State::HOLD_POSITION;
      }
      break;

    default:
      initializeSystoleState();
      break;
  }
}