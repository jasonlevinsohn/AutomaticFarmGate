#include <IRremote.h>

/*
  Remote Farm Gate
  Home Button is NEC: 5743C03F
  Back Button is NEC: 57436699
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


// Buttons to manually stop/start motor
int gateStartStopPin = 4;
int lockStartStopPin = 5;

const int analogGatePositionPin = 4;


int isGateMoving = false;
int isLockMoving = false;
int isGateButtonPressed = true;
int isLockButtonPressed = false;

int gateState = 1;

int gatePos = 0;

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
  
  //Button to stop and start actuator.
  pinMode(gateStartStopPin, INPUT_PULLUP);
  pinMode(lockStartStopPin, INPUT_PULLUP);
  
  
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
  // Start a Gate Sequence.
  signal = getIrRemoteSignal();
  if(signalReceived) {
    signalReceived = false;
    runGateSequence(signal); // &&&& BUILD THIS FUNCTION &&&&
  
  governActuatorMovingVars();
  
  //stopGateAt(600);  
  gatePos = analogRead(analogGatePositionPin);
  //Serial.println(gatePos);
    
  if(gatePressed) {
    gatePressed = false;
    
    Serial.print("Current State: ");
    Serial.println(gateState);
    Serial.print("Moving the gate in 3");
    delay(500);
    Serial.print(" 2");
    delay(500);
    Serial.println(" 1");
    delay(500);
    Serial.println("GO");
    delay(500);
    

    
    if(gateState == 1) {
      
      //Arduino Motor Shield
      digitalWrite(gateDirection, HIGH);
      digitalWrite(gateBrake, LOW);
      analogWrite(gateSpeed, speed5);
      
      digitalWrite(ledPin, HIGH);

      gateState = 2;
      
    } else if (gateState == 2) {
      
      digitalWrite(gateDirection, LOW);
      digitalWrite(gateBrake, HIGH);
      analogWrite(gateSpeed, 0);
      
      digitalWrite(ledPin, LOW);
            
      gateState = 3;

      
    } else if(gateState == 3) {
      
      //Arduino Motor Shield
      digitalWrite(gateDirection, LOW);
      digitalWrite(gateBrake, LOW); 
      analogWrite(gateSpeed, speed5); 
      
      digitalWrite(ledPin, HIGH);
      
      gateState = 4;
      
    
    } else if(gateState == 4) {

      digitalWrite(gateDirection, LOW);
      digitalWrite(gateBrake, HIGH);
      analogWrite(gateSpeed, 0); 
      
      digitalWrite(ledPin, LOW);
      
      
      gateState = 1;
      
    }
    
  }
  
  if(isLockMoving && lockPressed) {
    lockPressed = false;
  }
  
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
void governActuatorMovingVars() {
  
  isGateButtonPressed = digitalRead(gateStartStopPin);
  isLockButtonPressed = digitalRead(lockStartStopPin);
  
  
  // Change the Gate Moving Boolean when the button
  // is pressed.
  if(isGateButtonPressed) {
    gatePressed = true;
    Serial.print("Gate Button Pressed: ");
    Serial.println(isGateButtonPressed);
    
    // If the gate is stopped, let's move.
    // If the gate is moving, let's stop. 
    if(isGateMoving) {
      isGateMoving = false;
    } else {
      isGateMoving = true;
    }
    
    
    // Show Gate Position
    Serial.print("Gate Position: ");
    gatePos = analogRead(analogGatePositionPin);
    Serial.println(gatePos);

  }
  
  // Change the Lock Moving Boolean when the button
  // is pressed.
  if(isLockButtonPressed) {
    lockPressed = true;
    //Serial.println("Lock Button Pressed");
    
    // If the lock is stopped, let's move.
    // If the lock is moving, let's stop
    if(isLockMoving) {
      isLockMoving = false;
    } else {
      isLockMoving = true; 
    }

  }
  
}

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
