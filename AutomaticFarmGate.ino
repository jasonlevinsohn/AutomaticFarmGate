#include <IRremote.h>

/*
  Remote Farm Gate
  ----------------
  Home Button is NEC: 5743c13e
  Back Button is NEC: 57436699
  Reset Button is NEC: 57432dd2
  
  TODO:
  - Functions to extend / retract the lock upon opening and closing - DONE
  - Higher Functions to call extend/retract lower function called lock/unlock Gate
  - Function to position gate at exact position using Serial input
  
  NOTE: We might want the ability to control gate
  manually from the breadboard.  This is how.
  
  int gateStartStopPin = 4;
  pinMode(gateStartStopPin, INPUT_PULLUP);
  isGateButtonPressed = digitalRead(gateStartStopPin);
    
*/

// Gate Test Booleans
int GATE_ARM_TEST = true;

// IR Code Assignments
const String homeButton = "5743c03f";
const String backButton = "57436699";
const String resetButton = "57432dd2";

// Actuator Speeds
const int speed1 = 100,
          speed2 = 140,
          speed3 = 180,
          speed4 = 240,
          speed5 = 255;
    
// Actuator Positions (eg. 1023 / 6 = 171)
const int pos1  = 85,
          pos2  = 170,
          pos3  = 255,
          pos4  = 340,
          pos5  = 425,
          pos6  = 510,
          pos7  = 595,
          pos8  = 680,
          pos9  = 765,
          pos10 = 850,
          pos11 = 935;
          

// The map function is also a good alternative to this, but
// we've already done the later so..... what evs :)
// ALTERNATIVE: int range = map(gatePos, positionMin, positionMax, 0, 5);
   
   
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
// the actuators.  Don't use 0.
// It's used by the motor controller
// already to output the amperage.
const int analogGatePositionPin = 4;
const int analogLockPositionPin = 5;

int gatePos = 0;
int lockPos = 0;

int isGateMoving = false;
int isLockMoving = false;

// These are used to switch the
// motors on and off and insure
// the writes to turn them on/off
// is not repeated during loop
// cycles.
int gatePressed = false;
int lockPressed = false;


// We might need these variables, but it also
// might be overkill.  We are going to punt on
// this now, until we run into problems.
int gateUnlocked = false;
int gateLocked = false;


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
    if(signal == resetButton) {
      signalReceived = false;
      isLockMoving = true;
      isGateMoving = true;
      resetActuators();
    } else {
      signalReceived = false;
      changeGateState(signal); // &&&& BUILD THIS FUNCTION &&&&
    }
  }
  //governActuatorMovingVars();
  
  //stopGateAt(600);  
  //gatePos = analogRead(analogGatePositionPin);
  //Serial.println(gatePos);
  
}

void resetActuators() {
  
  
  int curPos = 0;
  
  Serial.println("Reseting Lock...");
  
  while(isLockMoving || isGateMoving) {
    
    while(isLockMoving) {
      
      if(GATE_ARM_TEST) {
        delay(500);
        Serial.println("\n ----- Gate Arm Test ----- ");
        Serial.println("\n ----- Setting Lock Open ----- ");
        delay(1000);
        digitalWrite(lockDirection, LOW);
        digitalWrite(lockBrake, LOW);
        analogWrite(lockSpeed, speed5);        
        
      } else {
        digitalWrite(lockDirection, HIGH);
        digitalWrite(lockBrake, LOW);
        analogWrite(lockSpeed, speed5);
      }
      
      
      curPos = analogRead(analogLockPositionPin);
      Serial.print(curPos);
      Serial.println(" Lock Position");
      isLockMoving = checkActuatorMotion(analogLockPositionPin);
      

    }    
    Serial.println("\n\nLock Reset\n\n");
    
    delay(2000);
    
    Serial.println("Reseting Gate...");
    while(isGateMoving) {
      digitalWrite(gateDirection, HIGH);
      digitalWrite(gateBrake, LOW);
      analogWrite(gateSpeed, speed5);
      
      curPos = analogRead(analogGatePositionPin);
      Serial.print(curPos);
      Serial.println(" Gate Position");
      isGateMoving = checkActuatorMotion(analogGatePositionPin);
    }
    
    Serial.println("\n\nGate Reset\n\n"); 
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


// Stops the lock from extending past the
// given pos.
/*
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
  
  
  
 
}
*/

void changeGateState(String signal) {
  
  
  // If the Home Button is pressed, and the 
  // gate is moving, stop the gate.
  Serial.println("Button Pressed...");
  Serial.println(signal);
  Serial.println(homeButton);
  Serial.println(backButton);
  Serial.println(isGateMoving);
  if(signal == homeButton && isGateMoving) {
    stopGate();
    Serial.println("Stopping the gate now");
  }
  
  // If the Home Button is pressed, and the 
  // gate is stopped, get the current position
  // and start moving at the cooresponding speed
  // in the opening direction.
  else if(signal == homeButton && !isGateMoving) {
    
    Serial.println("Opening the gate now");
    openTheGateIncrementally();  // &&&&& BUILD THIS FUNCTION &&&&&
    Serial.println("We opened the gate");
  }
  
  // If the Back Button is pressed, and the
  // gate is moving, stop the gate.
  else if(signal == backButton && isGateMoving) {
   stopGate(); 
   Serial.println("Stopping the gate now again");
  }
  
  // If the Back Button is pressed, and the
  // gate is stopped, get the current position
  // and start moving at the cooresponding speed
  // in the closing direction
  else if(signal == backButton && !isGateMoving) {
    closeTheGateIncrementally(); // &&&&& BUILD THIS FUNCTION &&&&& 
    Serial.println("Closing the gate now bitches");
  }
}

/*
  Closes the gate starting out slowly and gradually
  getting to full speed in the middle of the pistons
  reach.  After the center point in gradually slows
  down until it reaches the position set to align 
  with the lock.
*/
void closeTheGateIncrementally() {
  
  isGateMoving = true;
  int failSafeCounter = 0;
  int oneTime = true;
  
  // Turn on LED while we are moving.
  switchLED(isGateMoving);

  // Check to see if the position is the same
  // over 5 loops. This will mean the gate is
  // not moving anymore.  Release from the loop.
  while(isGateMoving) {
        
    // While loops are dangerous.  We need a fail safe
    // to break from the loop.
    failSafeCounter++;
    
    if(failSafeCounter == 3000) {
      Serial.println("Fail Safe hit 3000");
    }
    
    if(failSafeCounter == 7000) {
      Serial.println("Fail Safe hit 7000"); 
    }
    
    if(failSafeCounter > 10000) {
      Serial.println("Fail Safe Hit... Breaking Loop");
      isGateMoving = false;
    }
    
    gatePos = analogRead(analogGatePositionPin);
    Serial.print("Closing The gate: ");
    Serial.println(gatePos);
    
    if(gatePos < pos2) {
      Serial.println("HELLA MOVING----");
      moveGateClosed(speed1);
    } else if (gatePos < pos2) {
      Serial.println("HELLA MOVING---------");
      moveGateClosed(speed2);
    } else if (gatePos < pos3) {
      Serial.println("HELLA MOVING---------------");
      moveGateClosed(speed3);
    } else if (gatePos < pos4) {
      Serial.println("HELLA MOVING-----------------------");
      moveGateClosed(speed4);
    } else if (gatePos < pos5) {
      Serial.println("HELLA MOVING-------------------------------");
      moveGateClosed(speed5);
    } else if (gatePos < pos6) {
      Serial.println("HELLA MOVING-----------------------");
      moveGateClosed(speed4);
    } else if (gatePos < pos7) {
      Serial.println("HELLA MOVING---------------");
      moveGateClosed(speed3);
    } else if (gatePos < pos8) {
      Serial.println("HELLA MOVING---------");
      moveGateClosed(speed2);
    } else if (gatePos < pos9) {
      Serial.println("HELLA MOVING----");
      moveGateClosed(speed1);
      // Debug: Let's print the position now.
      if (oneTime) {
        oneTime = false; 
        Serial.println("Almost Closed,  Printing position....");
      }
      Serial.print(gatePos);
    }
      
      /* %%%%%%%%%  AFTER INITIAL TESTING %%%%%%%%%%%%
            Put the part to line up the gate with the lock here.
            We are also going to need to implement a serial input
            method to move the gate to an exact input position.
      
      // %%%%%%%%%  AFTER INITIAL TESTING %%%%%%%%%%%% */
    
    //Put this in the last moveGateOpen function
    isGateMoving = checkActuatorMotion(analogGatePositionPin);
 
  } // While Loop
  Serial.println("Past While Loop");
  
  // Turn off LED now that we've stopped.
  switchLED(isGateMoving);
  
  // After the gate is done closing. Lock it.
  if(GATE_ARM_TEST) {
    Serial.println("\n ----- Gate Arm Test ----- \n");
    Serial.println("----- Leaving Gate Unlocked ----- \n");
  } else {
    lockGate();  
  }
  
  
}

/*
  Opens the gate starting out slowly and gradually
  getting to full speed in the middle of the pistons
  reach.  After the center point in gradually slows
  down until it reaches the position set to align 
  with the lock.
*/
void openTheGateIncrementally() {
  
  isGateMoving = true;
  int failSafeCounter = 0;
  int oneTime = true;
  
  // Unlock the gate before moving it.
  if(GATE_ARM_TEST) {
    Serial.println("\n ----- Gate Arm Test ----- \n");
    Serial.println("----- No Action Required on Lock ----- \n");
  } else {
    unlockGate();  
  }

  
  // Turn on LED while we are moving.
  switchLED(isGateMoving);

  // Check to see if the position is the same
  // over 5 loops. This will mean the gate is
  // not moving anymore.  Release from the loop.
  Serial.println("Alright let's open this gate...");
  while(isGateMoving) {
    Serial.println("Uhh, yes sir opening");
        
    // While loops are dangerous.  We need a fail safe
    // to break from the loop.
    failSafeCounter++;
    
    if(failSafeCounter == 3000) {
      Serial.println("Fail Safe hit 3000");
    }
    
    if(failSafeCounter == 7000) {
      Serial.println("Fail Safe hit 7000"); 
    }
    
    if(failSafeCounter > 10000) {
      Serial.println("Fail Safe Hit... Breaking Loop");
      isGateMoving = false;
    }
    
    gatePos = analogRead(analogGatePositionPin);
    
    Serial.print("Gate Position: ");
    Serial.println(gatePos);
    // Now we start actually moving the gate YEAH :)
    
    if(gatePos > pos9) {
      Serial.print("Are we getting here: ");
      Serial.print(gatePos);
      Serial.print(" "); 
      Serial.println(pos9);
      moveGateOpen(speed1);
    } else if (gatePos > pos8) {
      moveGateOpen(speed2);
    } else if (gatePos > pos7) {
      moveGateOpen(speed3);
    } else if (gatePos > pos6) {
      moveGateOpen(speed4);
    } else if (gatePos > pos5) {
      moveGateOpen(speed5);
    } else if (gatePos > pos4) {
      moveGateOpen(speed4);
    } else if (gatePos > pos3) {
      moveGateOpen(speed3);
    } else if (gatePos > pos2) {
      moveGateOpen(speed2);
    } else if (gatePos > pos1) {
      moveGateOpen(speed1);
      // Debug: Let's print the position now.
      if (oneTime) {
        oneTime = false; 
        Serial.println("Almost Open,  Printing position....");
      }
      Serial.print(gatePos);
    }
      
    //Put this in the last moveGateOpen function
    isGateMoving = checkActuatorMotion(analogGatePositionPin);
 
  } // While Loop
  Serial.println("Past While Loop");
  
  // Turn off LED now that we've stopped.
  switchLED(isGateMoving);
  
  // 
  
}


/*
  Tests if the gate is in the same position over a period
  of 50ms.  This function is primarily used to turn the
  isGateMoving variable back to false, if the internal 
  switches in the actuators have stopped the piston at
  either end of the cylinder.
  Params: -> moving (boolean) is gate currently moving
  Returns:-> isSamePos (boolean) has gate moved over last 50 ms.
*/
int checkActuatorMotion(int actuatorPositionPin) {
  
    int isSamePos = true;
    int isStillMoving = true;
    int curPos, lastPos;
     
     // Let's get the position from before the first time delay.
     lastPos = analogRead(actuatorPositionPin);
    
    for(int i = 0; i < 5; i++) {
   // Serial.println("what is i: ");
   // Serial.println(i);
      // We check for isSamePos here because it
      // is true when counter is 0.  If at anytime
      // the gatePos does not equal the previous one
      // the gate is still moving and we don't need
      // to continue.
      if(isSamePos) {
        
        delay(10);
       
        // Get the current gate position
        curPos = analogRead(actuatorPositionPin);
        
        if(lastPos == curPos) {
          lastPos = curPos; 
          isSamePos = true;
          isStillMoving = false;
          Serial.print("CHECKING ACTUATOR MOTION: ATTEMPT ");
          Serial.print(i);
          Serial.println(" -> STOPPED");
        } else {
          lastPos = curPos;
          isSamePos = false;
          isStillMoving = true;
          Serial.print("CHECKING ACTUATOR MOTION: ATTEMPT ");
          Serial.print(i);
          Serial.println(" -> MOVING");
          // NOTE: We could probably use a break here instead.
          // It would be more performant.
        } 
      }
    }
    
    return isStillMoving;
}

// Turns the LED on or off
// Params -> onState (boolean) turn on or off
void switchLED(int onState) {
   if(onState) {
     digitalWrite(ledPin, HIGH);
   } else {
     digitalWrite(ledPin, LOW);
   }
}


// Move the gate toward the closed position.
// Params:
// rate: - speed to move the gate
void moveGateClosed(int rate) {
  
   // Set the moving state variable
   isGateMoving = true;
  
   // Move the Actuator  
   digitalWrite(gateDirection, HIGH);
   digitalWrite(gateBrake, LOW);
   analogWrite(gateSpeed, 255);
}

// Move the gate toward the open position.
// Params:
// rate: - speed to move the gate
void moveGateOpen(int rate) {
  
  // Set moving state variable
  isGateMoving = true;
  Serial.println("are we gonna come in here");
  Serial.print("What is the rate: ");
  Serial.println(rate);
  
  
  // Move the Actuator
  digitalWrite(gateDirection, LOW);
  digitalWrite(gateBrake, LOW); 
  analogWrite(gateSpeed, 255);
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
}


/*
  Move the lock actuator piston to the extended
  position.
*/
void extendLockActuator() {
  
  digitalWrite(lockDirection, HIGH);
  digitalWrite(lockBrake, LOW);
  analogWrite(lockSpeed, speed5);
}

/*
  Move the lock actuator piston to the retracted
  position
*/
void retractLockActuator() {
    
  digitalWrite(lockDirection, LOW);
  digitalWrite(lockBrake, LOW);
  analogWrite(lockSpeed, speed5);
  
}

/*
  Stop lock actuator
*/
void stopLock() {
  
  digitalWrite(lockDirection, LOW);
  digitalWrite(lockBrake, HIGH);
  analogWrite(lockSpeed, 0); 
  
}


/*
  Lock the gate.
*/
void lockGate() {
  
    int curPos = 0;
    int failSafeCounter = 0;
    
    // While loops are dangerous.  We need a fail safe
    // to break from the loop.
    failSafeCounter++;
    
    if(failSafeCounter == 3000) {
      Serial.println("Fail Safe hit 3000");
    }
    
    if(failSafeCounter == 7000) {
      Serial.println("Fail Safe hit 7000"); 
    }
    
    if(failSafeCounter > 10000) {
      Serial.println("Fail Safe Hit... Breaking Loop");
      isLockMoving = false;
    }
   
   // If we just started, run extend
   // function once.
   if(!isLockMoving) {
     isLockMoving = true;
     Serial.println("Extending the lock...");
     extendLockActuator(); 
   } 
   
   // Get the lock position. If we are
   // close to the end start checking
   // the position to see if the actuator
   // is still moving.
   while(isLockMoving) {
     curPos = analogRead(analogLockPositionPin);
     if (curPos > pos11) {
       Serial.println("Almost fully extended");
       isLockMoving = checkActuatorMotion(analogLockPositionPin);
     } 
   }
   
   stopLock();
   Serial.println("The Lock has stopped");
}

/*
  Unlock the gate.
*/
void unlockGate() {
  
  int curPos = 0;
  int failSafeCounter = 0;
  
  
  
  // While loops are dangerous.  We need a fail safe
  // to break from the loop.
  failSafeCounter++;
    
    if(failSafeCounter == 3000) {
      Serial.println("Fail Safe hit 3000");
    }
    
    if(failSafeCounter == 7000) {
      Serial.println("Fail Safe hit 7000"); 
    }
    
    if(failSafeCounter > 10000) {
      Serial.println("Fail Safe Hit... Breaking Loop");
      isLockMoving = false;
    }
  
  
 
  // If we just started, run retract
  // function once.
  if(!isLockMoving) {
    isLockMoving = true;
    Serial.println("Retracting the lock...");
    retractLockActuator();
  }
  
  // Get the lock position. If we are
  // close to the end of the start checking
  // the position to see if the actuator
  // is still moving.
  while(isLockMoving) {
    curPos = analogRead(analogLockPositionPin);
    Serial.println("What is cur: ");
    Serial.println(curPos);
    if (curPos < pos2) {
      Serial.println("Almost fully retracted");
      isLockMoving = checkActuatorMotion(analogLockPositionPin); 
    }
  }
  
  stopLock();
  Serial.println("The Lock is stopped");
     
}
