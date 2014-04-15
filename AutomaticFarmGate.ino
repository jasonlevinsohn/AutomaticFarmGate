/*
  Remote Farm Gate
*/
//LED Pins
int ledPin = 2;

//Motor Pins - Arduino Shield

int gateDirection = 12;
int gateSpeed = 3;
int gateBrake = 9;
int gateCurrent = 0;
int lockDirection = 13;
int lockSpeed = 11;
int lockBrake = 8;
int lockCurrent = 1;







int gateStartStopPin = 4;
int lockStartStopPin = 5;

int gatePositionPin = 6;
int lockPositionPin = 7;

int isGateMoving = false;
int isLockMoving = false;
int isGateButtonPressed = true;
int isLockButtonPressed = false;

int gateState = 1;

// These are used to switch the
// motors on and off and insure
// the writes to turn them on/off
// is not repeated during loop
// cycles.
int gatePressed = false;
int lockPressed = false;


void setup() {
  
  Serial.begin(9600);
  
  //LED
  pinMode(ledPin, OUTPUT);
  
  //Button to stop and start actuator.
  pinMode(gateStartStopPin, INPUT_PULLUP);
  pinMode(lockStartStopPin, INPUT_PULLUP);
  
  //Pins to get the gates current Position
  pinMode(gatePositionPin, INPUT);
  pinMode(lockPositionPin, INPUT);

  delay(1000);
  Serial.println("\nInitializing Gate Motor....");
  
  pinMode(gateDirection, OUTPUT);
  pinMode(gateBrake, OUTPUT);
  delay(400);
  
  Serial.println("Initiating Lock Motor....");
  pinMode(lockDirection, OUTPUT);
  pinMode(lockBrake, OUTPUT);
  delay(400);
  
  Serial.println("Remote Gate Initiated.");
 
  
}


void loop() {
  
  governActuatorMovingVars();
  Serial.print("Gate Moving:");
  Serial.println(isGateMoving);
  Serial.print("Gate Pressed: ");
  Serial.println(gatePressed);
 
  
  if(gatePressed) {
    gatePressed = false;
    
    Serial.print("Current State: ");
    Serial.println(gateState);
    Serial.print("Moving the gate in 3");
    delay(1000);
    Serial.print(" 2");
    delay(1000);
    Serial.println(" 1");
    delay(1000);
    Serial.println("GO");
    delay(1500);
    if(gateState == 1) {
      
      //Arduino Motor Shield
      digitalWrite(gateDirection, HIGH);
      digitalWrite(gateBrake, LOW);
      analogWrite(gateSpeed, 255);
      
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
      analogWrite(gateSpeed, 255); 
      
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
    Serial.println("Gate Button Pressed");
    
    // If the gate is stopped, let's move.
    // If the gate is moving, let's stop. 
    isGateMoving != isGateMoving;

  }
  
  // Change the Lock Moving Boolean when the button
  // is pressed.
  if(isLockButtonPressed) {
    lockPressed = true;
    Serial.println("Lock Button Pressed");
    
    // If the lock is stopped, let's move.
    // If the lock is moving, let's stop
    isLockMoving != isLockMoving;
  }
  
}