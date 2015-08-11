//#include <IRremote.h>
#include <BMSerial.h>
#include <RoboClaw.h>

/*
  Remote Farm Gate
  ----------------
  Home Button is NEC: 5743c13e
  Back Button is NEC: 57436699
  Reset Button is NEC: 57432dd2

  TODO:
  - Functions to extend / retract the lock upon opening and closing - DONE
  - Higher Functions to call extend/retract lower function called lock/unlock Gate - DONE
  - Function to position gate at exact position using Serial input
  
  NOTE: We might want the ability to control gate
  manually from the breadboard.  This is how.
  
  int gateStartStopPin = 4;
  pinMode(gateStartStopPin, INPUT_PULLUP);
  isGateButtonPressed = digitalRead(gateStartStopPin);
    
*/

// Controller Address
#define address 0x80


// Gate Test Booleans - Leaves the lock open
// for testing the arm.
int GATE_ARM_TEST = true;

// Radio Receiver Code Assignments
const String homeButton = "5743c03f";
const String backButton = "57436699";
const String resetButton = "57432dd2";

// We may need a different button to stop the gate, while moving.
// I'm afraid with out a debounce method in place, using the same code
// will call the stop function before it gets a chance to move at all.
const String playStopButton = "";

// RoboClaw Actuator Speeds
//const int speed1 = 8,
//          speed2 = 16,
//          speed3 = 32,
//          speed4 = 64,
//          speed5 = 127;
const int speed1 = 16,
          speed2 = 36,
          speed3 = 56,
          speed4 = 96,
          speed5 = 127;

// Actuator Positions (eg. 1023 / 6 = 171)
const int pos1  = 50,  // 15 from 35 (actual opened position)
          pos2  = 80,  // 30 from 50 
          pos3  = 140, // 60 from 80
          pos4  = 280, // 120 from 140
          pos5  = 445, // in the middle
          pos6  = 699, // 120 from 819
          pos7  = 819, // 60 from 879
          pos8  = 879, // 30 from 909
          pos9  = 909, // 15 from 924 (actual closed position)
          pos10 = 920;


// The map function is also a good alternative to this, but
// we've already done the later so..... what evs :)
// ALTERNATIVE: int range = map(gatePos, positionMin, positionMax, 0, 5);

// ########## PIN DEFINITIONS - BEGIN ########## 

// LED Pins
const int ledPin = 2;

// Button Pins
const int button1Pin = 8;

// Radio Receiver Pins
const int radioD1 = 9;
const int radioD2 = 10;
const int radioD3 = 11;
const int radioD4 = 12;

// Motor Controller Serial Comm Pins
// Swap S1/S2 (S2 Receive/ S1 Transmit)
const int S1 = 5;
const int S2 = 6;

// Z-Wave Switch Pin
const int ZWavePin = A3;
const int zwaveSwitchPin = 7;

//IRRemote Pin & Setup
//const int RECV_PIN = 6;
//IRrecv irrecv(RECV_PIN);
//decode_results results;
//int signalReceived = false;
//String signal;

//Motor Pins - Arduino Shield
/* const int gateCurrent = 0, */
/*           lockCurrent = 1, */
/*           gateSpeed = 3, */
/*           lockBrake = 8, */
/*           gateBrake = 9, */
/*           lockSpeed = 11, */
/*           gateDirection = 12, */
/*           lockDirection = 13; */


// Pin to get the current position of
// the actuators.  Don't use 0.
// It's used by the motor controller
// already to output the amperage.
const int analogGatePositionPin = A5;
const int analogLockPositionPin = A4;


// ########## PIN DEFINITIONS - END ########## 

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

boolean button1State = false;
boolean radioD1State = false;
boolean radioD2State = false;
boolean radioD3State = false;
boolean radioD4State = false;

int zwaveValue = 0;
boolean isZwaveOn = false;

// Setup RoboClaw Communication (Pins and timeout, 10ms)
// with serial comm. always connect arduino receive to roboclaw transmit
RoboClaw roboclaw(S2, S1, 10000);


void setup() {

  int currentGatePos = 0;

  // Initiate Serial Port
  Serial.begin(2400);

  // Initiate RoboClaw
  roboclaw.begin(9600);

  // Set LED Pin
  pinMode(ledPin, OUTPUT);

  // Set Button Pin
  pinMode(button1Pin, INPUT);
  
  // Set Radio Pins
  pinMode(radioD1, INPUT);
  pinMode(radioD2, INPUT);
  pinMode(radioD3, INPUT);
  pinMode(radioD4, INPUT);

  // Z-Wave Pin
  pinMode(zwaveSwitchPin, OUTPUT);


  // After coming online, check the state of the gate.
  // Set the Z-Wave component accordingly.
  delay(1000);
  currentGatePos = getGatePosition(50);

  if (currentGatePos > 900) {
    digitalWrite(zwaveSwitchPin, LOW);
    Serial.println("\n\nZ-Wave Switch Pin is LOW");
  } else {
    digitalWrite(zwaveSwitchPin, HIGH);
    Serial.println("\n\nZ-Wave Switch Pin is HIGH");
  }

  delay(1000);

  // IR Remote Setup
  // irrecv.enableIRIn();  // Start the receiver
  
  
  // Initialize Linear Actuators
  /* pinMode(gateDirection, OUTPUT); */
  /* pinMode(gateBrake, OUTPUT); */
 

  /* //%%%%%%%%%  IS THERE A CLEAR FUNCTION %%%%%%%%%% */
  /* //Serial.clear(); */
  /* Serial.println("Gate Actuator Initialized."); */

  /* pinMode(lockDirection, OUTPUT); */
  /* pinMode(lockBrake, OUTPUT); */
 
  /* Serial.println("Lock Actuator Initialized."); */
  delay(400);

  Serial.println("Remote Gate Activated.\n\n");

}


void loop() {

  int currentGatePosition = 0;
  int currentGateState = 0; // 0=Closed, 1=Open, 2=Interim
  int zwaveState = 2; // 0=Closed, 1=Open, 2=No Reading
  
  // Button Test
  // Serial.print("button 1 value: ");
  // Serial.println(digitalRead(button1Pin));

  /* button1State = digitalRead(button1Pin); */
  /* if (button1State) { */
  /*   digitalWrite(ledPin, HIGH); */
  /*   digitalWrite(13, HIGH); */

  /* } else { */
  /*   digitalWrite(ledPin, LOW); */
  /*   digitalWrite(13, LOW); */
  /* } */
  /* delay(400); */

  // Radio Receiver States
  radioD1State = digitalRead(radioD1);
  radioD2State = digitalRead(radioD2);
  radioD3State = digitalRead(radioD3);
  radioD4State = digitalRead(radioD4);


  // Z-Wave State
  zwaveValue = analogRead(ZWavePin);

  if (zwaveValue > 0) {
    zwaveState = 1;
  } else if (zwaveValue <= 0) {
    zwaveState = 0;
  } else {
    zwaveState = 2;
  }

  currentGatePosition = getGatePosition(50);
  /* Serial.print("Current Gate Position: "); */
  /* Serial.println(currentGatePosition); */
  // Gate is Open
  if (currentGatePosition < 50) {
    currentGateState = 1;  
  // Gate is Closed
  } else if (currentGatePosition > 900) {
    currentGateState = 0;
  // Gate is in an interim state
  } else {
    currentGateState = 2;
  }


  /* Serial.println("MAIN LOOP BEGINNING"); */
  /* Serial.print("D1: "); */
  /* Serial.println(radioD1State); */
  /* Serial.print("D2: "); */
  /* Serial.println(radioD2State); */
  /* Serial.print("D3: "); */
  /* Serial.println(radioD3State); */
  /* Serial.print("D4: "); */
  /* Serial.println(radioD4State); */

  /* Serial.print("Z-Wave State: "); */
  /* Serial.print(zwaveState); */
  /* Serial.print(" Current Gate State: "); */
  /* Serial.println(currentGateState); */
  
  // Radio Receiver
  if (radioD1State) {

    Serial.println("CLOSE THE GATE");
    changeGateState("close");
    delay(1000);
    tellZWaveGateStatus(false);

  } else if (radioD2State) {

    Serial.println("OPEN THE GATE");
    changeGateState("open");
    delay(1000);
    tellZWaveGateStatus(true);

  } else if (radioD3State) {
    
    printBatteryLevels(); 
    
  } else if (radioD4State) {
    Serial.println("STOP BUTTON PRESSED....  STOPPING MOTORS");
    stopGate();
  
  // Z-Wave Conditions
  // Open Gate if zwaveState is open(1) and Gate State is Closed(0)
  } else if (zwaveState == 1 && currentGateState == 0) {
    /* Serial.print("ZWave says open and Gate Says Closed"); */
    changeGateState("open");
  // Close Gate if zwaveState is closed(0) and Gate State is Open(1)
  } else if (zwaveState == 0 && currentGateState == 1) {
    /* Serial.println("ZWave says closed and Gate Says Open"); */
    changeGateState("close");

  } else {
    digitalWrite(ledPin, LOW);
  }
  // Get the signal if there is one.
  // Change the Current State of the Gate.
  /* signal = getIrRemoteSignal(); */
  /* if(signalReceived) { */
    
  /*   /1* checkSignalCode(signal); *1/ */
    
  /*   if(signal == resetButton) { */
  /*     signalReceived = false; */
  /*     isLockMoving = true; */
  /*     isGateMoving = true; */
  /*     resetActuators(); */
  /*   } else { */
  /*     signalReceived = false; */
  /*     changeGateState(signal); // &&&& BUILD THIS FUNCTION &&&& */
  /*   } */
  /* } */

}

void printBatteryLevels() {
   uint16_t main_battery;
   uint16_t logic_battery;
   uint16_t test_battery;

   unsigned char main_buf[2];
   unsigned char logic_buf[4];
   unsigned char test_buf[2];
   Serial.println("Voltage Levels: \n");
   
   main_battery = roboclaw.ReadMainBatteryVoltage(address);
   logic_battery = roboclaw.ReadLogicBattVoltage(address);
   main_buf[0] = (main_battery >> 8);
   main_buf[1] = main_battery;
   logic_buf[0] = (logic_battery >> 8);
   logic_buf[1] = (logic_battery >> 8);
   logic_buf[2] = logic_battery;   
   
   Serial.println(main_buf[0]);
   Serial.println(main_buf[1]);
   Serial.println("\n\n");
   Serial.println(logic_buf[0]);
   Serial.println(logic_buf[1]);
   Serial.println("One Mo Time");
   Serial.println(logic_buf[2]);
   Serial.println(logic_buf[3]);
   
   
   /* Serial.println(main_battery, HEX); */
   /* Serial.println(logic_battery, HEX); */

   delay(1000);

   
   // Serial.print((mainBattery >> 8), HEX);
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
        /* digitalWrite(lockDirection, LOW); */
        /* digitalWrite(lockBrake, LOW); */
        /* analogWrite(lockSpeed, speed5); */        
        retractLockActuator();
        
      } else {
        /* digitalWrite(lockDirection, HIGH); */
        /* digitalWrite(lockBrake, LOW); */
        /* analogWrite(lockSpeed, speed5); */
        /* moveGateClosed(speed5); */
        extendLockActuator();
      }
      
      curPos = analogRead(analogLockPositionPin);
      Serial.print(curPos);
      Serial.println(" Lock Position");
      isLockMoving = checkActuatorMotion(analogLockPositionPin);
      

    }    
    Serial.println("\n\nLock Reset\n\n");
    /* analogWrite(lockSpeed, 0); */
    stopLock();
    
    delay(2000);
    
    Serial.println("Reseting Gate...");
    while(isGateMoving) {
      /* digitalWrite(gateDirection, HIGH); */
      /* digitalWrite(gateBrake, LOW); */
      /* analogWrite(gateSpeed, speed5); */
      moveGateClosed(speed5);

      
      curPos = analogRead(analogGatePositionPin);
      Serial.print(curPos);
      Serial.println(" Gate Position");
      isGateMoving = checkActuatorMotion(analogGatePositionPin);
    }

    Serial.println("\n\nGate Reset\n\n");
    /* analogWrite(gateSpeed, 0); */
    stopGate();


  }

}

// Captures and decodes Remote Control Signal
//String getIrRemoteSignal() {
//  String signal;
//  if (irrecv.decode(&results)) {
//     Serial.println("Remote Control Signal: ");
//     signal = formatDecodeResult(&results);
//     irrecv.resume(); // Receive the next value
//     signalReceived = true;
//  }
//  return signal;
//}

// Formats and outputs the content of the decode result
// object to the serial port.
//String formatDecodeResult(const decode_results* results) {
//  String value;
//  const int protocol = results->decode_type;
//  Serial.print("Protocol: ");
//
//  if (protocol == UNKNOWN) {
//    Serial.println("not recognized");
//  } else {
//    if (protocol == NEC) {
//      Serial.println("NEC");
//    } else if (protocol == RC5) {
//      Serial.println("RC5");
//    } else if (protocol == RC6) {
//      Serial.println("RC6");
//    } else if (protocol == SONY) {
//      Serial.println("SONY");
//    }
//    Serial.print("Value: ");
//    Serial.print(results->value, HEX);
//    Serial.print(" (");
//    Serial.print(results->bits, DEC);
//    Serial.println(" bits)");
//    value = String(results->value, HEX);
//    Serial.print("What is value: ");
//    Serial.println(value);
//  }
//  return value;
//}


void changeGateState(String signal) {

  // If the Home Button is pressed, and the
  // gate is stopped, get the current position
  // and start moving at the cooresponding speed
  // in the opening direction.
  if(signal == "open" && !isGateMoving) {
    openTheGateIncrementally();
  }

  // If the Back Button is pressed, and the
  // gate is stopped, get the current position
  // and start moving at the cooresponding speed
  // in the closing direction
  else if(signal == "close" && !isGateMoving) {
    closeTheGateIncrementally();
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
  int sampleSize = 50;
  int sampleData[sampleSize];
  
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
    
    /* gatePos = analogRead(analogGatePositionPin); */

    // Get a sampling of the data
    for (int i = 0; i < sampleSize; i++) {
        sampleData[i] = analogRead(analogGatePositionPin);
    }

    gatePos = getMedian(sampleData, sampleSize);

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
    
    // Check the alarm button on radio remote
    // while the gate is moving to stop it, if necessary.  
    checkForStopButtonInvocation();
 
  } // While Loop
  Serial.println("Past While Loop");
  stopGate();
  
  // Turn off LED now that we've stopped.
  switchLED(isGateMoving);

  // After the gate is done closing. Lock it.
  // Make sure the gate is fully closed before locking gate
  Serial.println("WHAT IS CLOSING STATUS POSITION:");
  Serial.println(gatePos);
  if(!GATE_ARM_TEST && gatePos > 920) {
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
  int sampleSize = 50;
  int sampleData[sampleSize];
  
  // Unlock the gate before moving it.
  if(!GATE_ARM_TEST) {
    unlockGate();
  }
  
  
  // Turn on LED while we are moving.
  switchLED(isGateMoving);

  // Check to see if the position is the same
  // over 5 loops. This will mean the gate is
  // not moving anymore.  Release from the loop.
  Serial.println("Alright let's open this gate...");
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
    
    /* gatePos = analogRead(analogGatePositionPin); */

    // Get a sampling of the data
    for (int i = 0; i < sampleSize; i++) {
        sampleData[i] = analogRead(analogGatePositionPin);
    }

    gatePos = getMedian(sampleData, sampleSize);
    
    Serial.print("Gate Position: ");
    Serial.println(gatePos);
    // Now we start actually moving the gate YEAH :)
    
    if(gatePos > pos9) {
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
    
    // Check the for the alarm button radio button pressed
    // while the gate is moving to stop it, if necessary.  
    checkForStopButtonInvocation();
 
  } // While Loop
  Serial.println("Past While Loop");
  stopGate();
  
  // Turn off LED now that we've stopped.
  switchLED(isGateMoving);
  
  // Tell Z-Wave the Gate is Open
  /* tellZWaveGateStatus(true); */
  
}

// Used to check for the stop button being pressed.  Interrupts
// the opening and closing of the gate.  Should be placed at the
// end of the while loop for isGateOpen in the open/close functions.
void checkForStopButtonInvocation() {
  radioD4State = digitalRead(radioD4);
  if (radioD4State) {
    stopGate();
  }
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
    int sampleSize = 75;
    int sampleData[sampleSize];

    // Get a sampling of the data
    for (int i = 0; i < sampleSize; i++) {
        sampleData[i] = analogRead(actuatorPositionPin);
    }

    // Get the median of the sample data.  No need to clear
    // the array.  We will just write over the previous values.
    lastPos = getMedian(sampleData, sampleSize);
     
    // Let's get the position from before the first time delay.
    /* lastPos = analogRead(actuatorPositionPin); */
    
    for(int j = 0; j < 5; j++) {
      // We check for isSamePos here because it
      // is true when counter is 0.  If at anytime
      // the gatePos does not equal the previous one
      // the gate is still moving and we don't need
      // to continue.
      if(isSamePos) {
        
        delay(10);
       
        for (int k = 0; k < sampleSize; k++) {
            sampleData[k] = analogRead(actuatorPositionPin);
        }

        curPos = getMedian(sampleData, sampleSize);
        
        if(lastPos == curPos) {
          lastPos = curPos; 
          isSamePos = true;
          isStillMoving = false;
          Serial.print("CHECKING ACTUATOR MOTION: ATTEMPT ");
          Serial.print(j);
          Serial.print(" POSITION: ");
          Serial.print(curPos);
          Serial.println(" -> STOPPED");
        } else {
          lastPos = curPos;
          isSamePos = false;
          isStillMoving = true;
          Serial.print("CHECKING ACTUATOR MOTION: ATTEMPT ");
          Serial.print(j);
          Serial.print(" POSITION: ");
          Serial.print(curPos);
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
   /* digitalWrite(gateDirection, HIGH); */
   /* digitalWrite(gateBrake, LOW); */
   /* analogWrite(gateSpeed, 255); */
   Serial.print("Closing Gate: ");
   Serial.println(rate);
   roboclaw.ForwardM1(address,rate);
}

// Move the gate toward the open position.
// Params:
// rate: - speed to move the gate
void moveGateOpen(int rate) {
  
  // Set moving state variable
  isGateMoving = true;
  
  // Move the Actuator
  /* digitalWrite(gateDirection, LOW); */
  /* digitalWrite(gateBrake, LOW); */ 
  /* analogWrite(gateSpeed, 255); */
  Serial.print("Opening Gate: ");
  Serial.println(rate);
  roboclaw.BackwardM1(address,rate);
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
  /* digitalWrite(gateDirection, LOW); */
  /* digitalWrite(gateBrake, LOW); */
  /* analogWrite(gateSpeed, 0); */ 
  Serial.println("Stopping Gate");
  /* roboclaw.ForwardBackwardM1(address,0); */
  roboclaw.ForwardM1(address, 0);
  roboclaw.BackwardM1(address, 0);
}


/*
  Move the lock actuator piston to the extended
  position.
*/
void extendLockActuator() {
  
  /* digitalWrite(lockDirection, HIGH); */
  /* digitalWrite(lockBrake, LOW); */
  /* analogWrite(lockSpeed, speed5); */
  roboclaw.ForwardM1(address, 128);
}

/*
  Move the lock actuator piston to the retracted
  position
*/
void retractLockActuator() {
    
  /* digitalWrite(lockDirection, LOW); */
  /* digitalWrite(lockBrake, LOW); */
  /* analogWrite(lockSpeed, speed5); */
  roboclaw.BackwardM2(address, 128);
  
}

/*
  Stop lock actuator
*/
void stopLock() {
  
  /* digitalWrite(lockDirection, LOW); */
  /* digitalWrite(lockBrake, HIGH); */
  /* analogWrite(lockSpeed, 0); */ 
  roboclaw.ForwardBackwardM2(address, 0);
  
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
     if (curPos > pos10) {
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

/* void checkSignalCode(String s) */ 
/*  if (s == homeButton || s == backButton || s == resetButton) { */
/*   Serial.println("IR CODE: MATCH"); */
/*  } else { */
/*   Serial.println("IR CODE: MATCH NOT FOUND"); */
/*   Serial.print("Reset Button is: "); */
/*   Serial.println(resetButton); */
/*   Serial.print("Home Button is: "); */
/*   Serial.println(homeButton); */
/*   Serial.print("Back Button is: "); */
/*   Serial.println(backButton); */
/*   Serial.print("Code Entered: "); */
/*   Serial.println(s); */
/*  } */ 
/* } */

int compare_int( const void* a, const void* b )
{
    if( *(int*)a == *(int*)b ) return 0;
    return *(int*)a < *(int*)b ? -1 : 1;
}

int getMedian(int *the_array, int size)
{

    int median;
    
    qsort( the_array, size, sizeof(int), compare_int );

    median = size / 2;

    return the_array[median];

}

int getGatePosition(int sampleSize)
{
    int _gatePosition = 0;
    int sampleData[sampleSize];

    // Get a sampling of the data
    for (int i = 0; i < sampleSize; i++) {
        sampleData[i] = analogRead(analogGatePositionPin);
    }

    _gatePosition = getMedian(sampleData, sampleSize);

    return _gatePosition;
}

void tellZWaveGateStatus(boolean isGateOpen) {
    if (isGateOpen) {
        digitalWrite(zwaveSwitchPin, HIGH);
        Serial.println("Z-Wave Switch Pin is HIGH");
    } else {
        digitalWrite(zwaveSwitchPin, LOW);
        Serial.println("Z-Wave Switch Pin is LOW");
    }

    Serial.print("Switching Z-Wave to: ");
    Serial.println(isGateOpen);

    delay(2000);
}
