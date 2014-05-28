#include <IRremote.h>

/*
  Remote Farm Gate
  ----------------
  Home Button is NEC: 5743C03F
  Back Button is NEC: 57436699
  
  
  NOTE: We might want the ability to control gate
  manually from the breadboard.  This is how.
  
  int gateStartStopPin = 4;
  pinMode(gateStartStopPin, INPUT_PULLUP);
  isGateButtonPressed = digitalRead(gateStartStopPin);
    
*/

const int speed1 = 50,
    speed2 = 100,
    speed3 = 150,
    speed4 = 200,
    speed5 = 255;
    
//LED Pins
const int ledPin = 2;

//IRRemote Pin & Setup
const int RECV_PIN = 6;
IRrecv irrecv(RECV_PIN);
decode_results results;
int signalReceived = false;
String signal;

//Motor Pins - Arduino Shield
const int gateCurrent = 0,
          lockCurrent = 1,
          gateSpeed = 3,
          lockBrake = 8,
          gateBrake = 9,
          lockSpeed = 11,
          gateDirection = 12,
          lockDirection = 13;


// Pin to get the current position of 
// the gate actuator.  Don't use 0.
// It's used by the motor controller
// already to output the amperage.
const int analogGatePositionPin = 4;
int gatePos = 0;

int isGateMoving = false;
int isLockMoving = false;



// These are used to switch the
// motors on and off and insure
// the writes to turn them on/off
// is not repeated during loop
// cycles.
int gatePressed = false;
int lockPressed = false;


void setup() {
  
  // Initiate Serial Port
  Serial.begin(9600);
  
  // Set LED Pin
  pinMode(ledPin, OUTPUT);
  
  // IR Remote Setup
  irrecv.enableIRIn();  // Start the receiver
  
  
  // Initialize Linear Actuators
  pinMode(gateDirection, OUTPUT);
  pinMode(gateBrake, OUTPUT);
  
  //%%%%%%%%%  IS THERE A CLEAR FUNCTION %%%%%%%%%%
  //Serial.clear();
  Serial.println("Gate Actuator Initialized\n\n");
  
  pinMode(lockDirection, OUTPUT);
  pinMode(lockBrake, OUTPUT);
  
  Serial.println("Lock Actuator Initialized\n\n");
  delay(400);
  Serial.println("Remote Gate Activated.");
 
  
}


void loop() {
  
  // Get the signal if there is one.
  // Change the Current State of the Gate.
  signal = getIrRemoteSignal();
  if(signalReceived) {
    signalReceived = false;
    changeGateState(signal); // &&&& BUILD THIS FUNCTION &&&&
  }
  //governActuatorMovingVars();
  
  //stopGateAt(600);  
  //gatePos = analogRead(analogGatePositionPin);
  //Serial.println(gatePos);
  
}

// Captures and decodes Remote Control Signal
String getIrRemoteSignal() {
  String signal;
  if (irrecv.decode(&results)) {
     Serial.println("Remote Control Signal: ");
     signal = formatDecodeResult(&results);
     irrecv.resume(); // Receive the next value 
     signalReceived = true;
  }
  return signal;
}

// Formats and outputs the content of the decode result
// object to the serial port.
String formatDecodeResult(const decode_results* results) {
  String value;
  const int protocol = results->decode_type;
  Serial.print("Protocol: ");
  
  if (protocol == UNKNOWN) {
    Serial.println("not recognized");
  } else {
    if (protocol == NEC) {
      Serial.println("NEC"); 
    } else if (protocol == RC5) {
      Serial.println("RC5"); 
    } else if (protocol == RC6) {
      Serial.println("RC6"); 
    } else if (protocol == SONY) {
      Serial.println("SONY"); 
    }
    Serial.print("Value: ");
    Serial.print(results->value, HEX);
    Serial.print(" (");
    Serial.print(results->bits, DEC);
    Serial.println(" bits)");
    value = String(results->value, HEX);
  }
  return value;
}

// Sets and governs the global actuator moving 
// variables which hold the moving state.
// TODO:
  // This function should also check the position of
  // the actuators and set the moving booleans
  // accordingly.
//void governActuatorMovingVars() {
//  
//  isGateButtonPressed = digitalRead(gateStartStopPin);
//  isLockButtonPressed = digitalRead(lockStartStopPin);
//  
//  
//  // Change the Gate Moving Boolean when the button
//  // is pressed.
//  if(isGateButtonPressed) {
//    gatePressed = true;
//    Serial.print("Gate Button Pressed: ");
//    Serial.println(isGateButtonPressed);
//    
//    // If the gate is stopped, let's move.
//    // If the gate is moving, let's stop. 
//    if(isGateMoving) {
//      isGateMoving = false;
//    } else {
//      isGateMoving = true;
//    }
//    
//    
//    // Show Gate Position
//    Serial.print("Gate Position: ");
//    gatePos = analogRead(analogGatePositionPin);
//    Serial.println(gatePos);
//
//  }
//  
//  // Change the Lock Moving Boolean when the button
//  // is pressed.
//  if(isLockButtonPressed) {
//    lockPressed = true;
//    //Serial.println("Lock Button Pressed");
//    
//    // If the lock is stopped, let's move.
//    // If the lock is moving, let's stop
//    if(isLockMoving) {
//      isLockMoving = false;
//    } else {
//      isLockMoving = true; 
//    }
//
//  }
//  
//}

// Stops the lock from extending past the
// given pos.
void stopGateAt(int pos) {

  gatePos = analogRead(analogGatePositionPin);
  
  //Serial.print("Gate Position: ");
  
  if(isGateMoving) {
    delay(100);
    
    // Only look at the reading if it is below 100. 
    // This might be due to voltage drop because of
    // the resistance of me using 3 feet of wire between
    // the actuator and controller.
    //if(gatePos < 200) {
      if(gatePos < 200) {
       analogWrite(gateSpeed, speed5);
      }
      else if (gatePos > 750) {
       analogWrite(gateSpeed, speed5);
      } 
      else {
        analogWrite(gateSpeed, speed5);
      }
    //}
   
    Serial.println(gatePos);
    
   
  }
  
  
  
  if(gatePos > pos) {
    
   /* Serial.println("----------------");
    Serial.println("Motion Interuppted");
    Serial.print("Gate Position: ");
    Serial.println(gatePos);
    Serial.println("----------------");   */
  }
}

void changeGateState(String signal) {
  
  // If the Home Button is pressed, and the 
  // gate is moving, stop the gate.
  
  // If the Home Button is pressed, and the 
  // gate is stopped, get the current position
  // and start moving at the cooresponding speed
  // in the opening direction.
  
  // If the Back Button is pressed, and the
  // gate is moving, stop the gate.
  
  // If the Back Button is pressed, and the
  // gate is stopped, get the current position
  // and start moving at the cooresponding speed
  // in the closing direction
}


// Move the gate toward the open position.
// Params:
// rate: - speed to move the gate
void moveOpen(int rate) {
  
   // Set the moving state variable
   isGateMoving = true;
  
   // Move the Actuator  
   digitalWrite(gateDirection, HIGH);
   digitalWrite(gateBrake, LOW);
   analogWrite(gateSpeed, rate);
   
   // Turn on the LED to signal movement.   
   digitalWrite(ledPin, HIGH);
}

// Move the gate toward the closed position.
// Params:
// rate: - speed to move the gate
void moveClosed(int rate) {
  
  // Set moving state variable
  isGateMoving = true;
  
  // Move the Actuator
  digitalWrite(gateDirection, LOW);
  digitalWrite(gateBrake, LOW); 
  analogWrite(gateSpeed, rate);
  
  // Turn on the LED to signal movement.    
  digitalWrite(ledPin, HIGH);
  
}

// Stop the gate
// TODO: Incrementally slow the gate down rather
// than a jolting stop.  Then again, the most likely
// reason for wanting to stop the gate before the
// completed action is most likely for an emergency.
void stopGate() {
  
  // Set the moving state variable
  isGateMoving = false;
  
  // Stop the Actuator.
  digitalWrite(gateDirection, LOW);
  digitalWrite(gateBrake, HIGH);
  analogWrite(gateSpeed, 0); 
      
  // Turn off the LED to signal stopped.    
  digitalWrite(ledPin, LOW);
  
}


