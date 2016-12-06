//#include <IRremote.h>
#include <BMSerial.h>
#include <RoboClaw.h>
// #include <SoftwareSerial.h>

// 434 RadioHead Transceiver Library
//#include <RH_ASK.h>
//#include <SPI.h> // Not used but needed to compile RadioHead


/*
  
  TODO:
  - For the close and lock function.  The return value of the position
    of the arm seems to fluctuate even when stopped.  We need it to hit
    an exact point before engaging the lock.  Maybe the reason it is
    fluctuationin is because the motor controller has the actuator still
    moving.  Maybe we could stop the actuator using the motor controller
    function when the arm actuator gets to a specific position where we
    want to fire the lock.
  - Function to position gate at exact position using Serial input
  
  NOTE: We might want the ability to control gate
  manually from the breadboard.  This is how.
  
  int gateStartStopPin = 4;
  pinMode(gateStartStopPin, INPUT_PULLUP);
  isGateButtonPressed = digitalRead(gateStartStopPin);
    
*/

// Controller Address
#define address 0x80

// Position we want Arm Actuator to be above before firing lock.
const int CHECK_CLOSE_GATE_POS_TO_FIRE_LOCK = 885;

// Sensitivity for check if the arm is moving or not. (2 - Not Sensitive || 5+ very sensitive)
const int MOVEMENT_CHECK_SENSITIVITY = 3;

// Allows values to be truthy and match if not exactly eqaul by this deviation.
const int MOVEMENT_CHECK_DEVIATION = 0;


// Gate Test Booleans - Leaves the lock open
// for testing the arm.
int GATE_ARM_TEST = true;

// Open for a vehicle = true
// Open for a person = false
int FULL_OPEN = false;

// RoboClaw Actuator Speeds
const int speed1 = 8,
          speed2 = 16,
          speed3 = 32,
          speed4 = 64,
          speed5 = 127;
const int open_speed1 = 8,
          open_speed2 = 16,
          open_speed3 = 32,
          open_speed4 = 64,
          open_speed5 = 127;
const int close_speed1 = 8,
          close_speed2 = 16,
          close_speed3 = 32,
          close_speed4 = 64,
          close_speed5 = 127;
          
//const int speed1 = 16,
//          speed2 = 36,
//          speed3 = 56,
//          speed4 = 56,
//          speed5 = 56;
          /* speed4 = 96, */
          /* speed5 = 127; */

// Actuator Positions (eg. 1023 / 6 = 171)
const int pos1  = 50,  // 15 from 35 (actual opened position)
          pos2  = 80,  // 30 from 50 
          pos3  = 140, // 60 from 80
          pos4  = 180, // 120 from 140
          pos5  = 445, // in the middle
          pos6  = 775, // 120 from 819
          pos7  = 819, // 60 from 879
          pos8  = 879, // 30 from 909
          pos9  = 909, // 15 from 924 (actual closed position)
          pos10 = 920;


// The map function is also a good alternative to this, but
// we've already done the later so..... what evs :)
// ALTERNATIVE: int range = map(gatePos, positionMin, positionMax, 0, 5);

// ########## PIN DEFINITIONS - BEGIN ########## 

// LED Pins
const int ledPin = 13;

// Button Pins
const int button1Pin = 8;

// Radio Receiver Pins
const int radioD1 = 9;
const int radioD2 = 8;
const int radioD3 = 11;
const int radioD4 = 12;

// Motor Controller Serial Comm Pins
// Swap S1/S2 (S2 Receive/ S1 Transmit)
const int S1 = 5;
const int S2 = 6;

// Z-Wave Switch Pin
//const int ZWavePin = A3;
//const int zwaveSwitchPin = 7;

// RadioHead Transmitter Pin
//const int radioHeadTransPin = 7;

// Xbee Communication
BMSerial XBee(2, 3); // Arduino RX, TX (Xbee Dout, Din)
int xBeeCounter = 0;



// Pin to get the current position of
// the actuators.  Don't use 0.
// It's used by the motor controller
// already to output the amperage.
const int analogGatePositionPin = A2;
const int analogLockPositionPin = A1;

// Voltage Divider Pins
const int batteryVoltageInputPin = A4;
const int solarChargerVoltageInputPin = A6;


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

//int zwaveValue = 0;
//boolean isZwaveOn = false;


// Voltage Divider Variables

float batteryVin = 0.0;
float solarVin = 0.0;
int voltageIntervalSeconds = 20;
unsigned long getVoltageInterval = voltageIntervalSeconds * 1000; // Get the voltage every n cycles.
unsigned long voltageTimestamp = millis();



// Setup RoboClaw Communication (Pins and timeout, 10ms)
// with serial comm. always connect arduino receive to roboclaw transmit
RoboClaw roboclaw(S2, S1, 10000);

// Setup RadioHead Driver
// We have to used the constructor because the default is Pin 12
// 1st param is speed
// 2nd param is receiver pin
// 3rd param is transmitter pin
// RH_ASK radioHeadDriver(2000, 11, 7);

// Char Array to Transmit
char batVoltArray[10];


void setup() {

  int currentGatePos = 0;

  // Initiate Serial Port
  Serial.begin(2400);

  // Initiate RoboClaw
  roboclaw.begin(9600);
  
  // Initiate Xbee
  XBee.begin(9600);

  // Set LED Pin
  pinMode(ledPin, OUTPUT);

  // Set Button Pin
  pinMode(button1Pin, INPUT);
  
  // Set Radio Pins
  pinMode(radioD1, INPUT);
  pinMode(radioD2, INPUT);
  pinMode(radioD3, INPUT);
  pinMode(radioD4, INPUT);
  
  // Setup RadioHead Driver
//  if (!radioHeadDriver.init()) {
//   Serial.println(F("RadioHead Driver initialization failed")); 
//  } else {
//    Serial.println(F("RadioHead Driver initialized"));
//  }
  
  

  // Z-Wave Pin
//  pinMode(zwaveSwitchPin, OUTPUT);

//  if (ZWAVE_ON) {
//    // After coming online, check the state of the gate.
//    // Set the Z-Wave component accordingly.
//    delay(1000);
//    currentGatePos = getGatePosition(50);
//    Serial.print("Current Gate Position: ");
//    Serial.println(currentGatePos);
//  
//    if (currentGatePos > 900) {
//      digitalWrite(zwaveSwitchPin, LOW);
//      Serial.println("\n\nZ-Wave Switch Pin is LOW");
//    } else {
//      digitalWrite(zwaveSwitchPin, HIGH);
//      Serial.println("\n\nZ-Wave Switch Pin is HIGH");
//    }
// 
//    delay(2000);
//  }
  
  // Voltage Divider Pins
  
  pinMode(batteryVoltageInputPin, INPUT);
  pinMode(solarChargerVoltageInputPin, INPUT);
  
  delay(400);

  Serial.println(F("Remote Gate Activated.\n\n"));

}


void loop() {
  char data;
  int currentGatePosition = 0;
  int currentGateState = 0; // 0=Closed, 1=Open, 2=Interim
//  int zwaveState = 2; // 0=Closed, 1=Open, 2=No Reading

  

  // Radio Receiver States
  radioD1State = digitalRead(radioD1);
  radioD2State = digitalRead(radioD2);
  radioD3State = digitalRead(radioD3);
  radioD4State = digitalRead(radioD4);

//  if (ZWAVE_ON) {
//    // Z-Wave State
//    zwaveValue = analogRead(ZWavePin);
//  
//    //  Serial.print("\nZWave Value: ");
//    //  Serial.println(zwaveValue);
//  
//    if (zwaveValue > 2) {
//      zwaveState = 1;
//    } else if (zwaveValue <= 1) {
//      zwaveState = 0;
//    } else {
//      zwaveState = 2;
//    }
//  }


  // Gate State
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
  // Serial.print("Current Gate Position: ");
  // Serial.println(currentGatePosition);
  // Serial.print("Current Gate State: ");
  // Serial.println(currentGateState);

  // Command AFG through Xbee Test
  if (XBee.available()) {
    data = XBee.read();
    Serial.println("Data from Xbee: ");
    Serial.println(data);
    if(data == 'c') {
     switchLED(true);
      delay(100);
     switchLED(false);
      delay(100);
     switchLED(true);
      delay(100);
     switchLED(false);
     Serial.print("Current Gate State: ");
     Serial.println(currentGateState);
     XBee.println("closing");
     changeGateState("close");
    } else if (data == 'o') {
     switchLED(true);
      delay(500);
     switchLED(false);
      delay(500);
     switchLED(true);
      delay(500);
     switchLED(false);
     XBee.println("opening");
     changeGateState("open");
    } else if (data == 'x') {
        //Yes, we are online.
        Serial.println("Reporting Gate State");
        Serial.print("Gate state is: ");
        Serial.print(currentGateState);
        Serial.print(" Position is: ");
        Serial.println(currentGatePosition);
        XBee.println("online");
        if (currentGateState == 1) {
            XBee.println("open");
        } else if (currentGateState == 0) {
            XBee.println("closed");
        } else {
            XBee.println("status unknown");
        }
    } else {
     switchLED(true);
     delay(2000);
     switchLED(false);
    }
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

    Serial.println(F("CLOSE THE GATE"));
    changeGateState("close");
    delay(1000);
    
//    if (ZWAVE_ON) {
//      tellZWaveGateStatus(false);
//    }
  } else if (radioD2State) {

    Serial.println(F("OPEN THE GATE"));
    changeGateState("open");
    delay(1000);
    
//    if (ZWAVE_ON) {
//      tellZWaveGateStatus(true);
//    }
  } else if (radioD3State) {
    
    resetActuators();
    
  } else if (radioD4State) {
    Serial.println(F("STOP BUTTON PRESSED....  STOPPING MOTORS"));
    stopGate();
  
  // Z-Wave Conditions
  // Open Gate if zwaveState is open(1) and Gate State is Closed(0)
//  } else if (ZWAVE_ON && zwaveState == 1 && currentGateState == 0) {
//    Serial.println("ZWave says open and Gate Says Closed");
//    /* changeGateState("open"); */
//  // Close Gate if zwaveState is closed(0) and Gate State is Open(1)
//  } else if (ZWAVE_ON && zwaveState == 0 && currentGateState == 1) {
//    Serial.println("ZWave says closed and Gate Says Open");
//    /* changeGateState("close"); */

  } else {
    digitalWrite(ledPin, LOW);
  }
  
  // Get and Print the Voltages for the battery and solar charger
  printVoltages(getVoltageInterval);
} // END Loop




void resetActuators() {
 

  int curPos = 0;
  String signal;
  
  Serial.println(F("Reseting Lock..."));
  isLockMoving = true;
  isGateMoving = true;

  while(isLockMoving || isGateMoving) {

    while(isLockMoving) {
      
      if(GATE_ARM_TEST) {
        delay(500);
        Serial.println(F("\n ----- Gate Arm Test ----- "));
        Serial.println(F("\n ----- Setting Lock Open ----- "));
        delay(1000);
        /* digitalWrite(lockDirection, LOW); */
        /* digitalWrite(lockBrake, LOW); */
        /* analogWrite(lockSpeed, speed5); */        
        retractLockActuator();
        signal = "open";
        
      } else {
        /* digitalWrite(lockDirection, HIGH); */
        /* digitalWrite(lockBrake, LOW); */
        /* analogWrite(lockSpeed, speed5); */
        /* moveGateClosed(speed5); */
        extendLockActuator();
        signal = "close";
      }
      
      curPos = analogRead(analogLockPositionPin);
      Serial.print(curPos);
      Serial.println(F(" Lock Position"));
      isLockMoving = checkActuatorMotion(analogLockPositionPin, signal);
      

    }    
    Serial.println(F("\n\nLock Reset\n\n"));
    /* analogWrite(lockSpeed, 0); */
    stopLock();
    
    delay(2000);
    
    Serial.println(F("Reseting Gate..."));
    while(isGateMoving) {
      /* digitalWrite(gateDirection, HIGH); */
      /* digitalWrite(gateBrake, LOW); */
      /* analogWrite(gateSpeed, speed5); */
      moveGateClosed(speed5);

      
      curPos = analogRead(analogGatePositionPin);
      Serial.print(curPos);
      Serial.println(F(" Gate Position"));
      isGateMoving = checkActuatorMotion(analogGatePositionPin, "close");
    }

    Serial.println(F("\n\nGate Reset\n\n"));
    /* analogWrite(gateSpeed, 0); */
    stopGate();

    // Tell Z-Wave the Gate is closed
//    delay(1000);
//    tellZWaveGateStatus(false);


  }

  Serial.println(F("Lock and Gate Reset Complete"));

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
    openTheGateIncrementally(signal);
  }

  // If the Back Button is pressed, and the
  // gate is stopped, get the current position
  // and start moving at the cooresponding speed
  // in the closing direction
  else if(signal == "close" && !isGateMoving) {
    closeTheGateIncrementally(signal);
  }
}

/*
  Closes the gate starting out slowly and gradually
  getting to full speed in the middle of the pistons
  reach.  After the center point in gradually slows
  down until it reaches the position set to align
  with the lock.
  @Param: signal - "open or close"
*/
void closeTheGateIncrementally(String signal) {

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
      Serial.println(F("Fail Safe hit 3000"));
    }

    if(failSafeCounter == 7000) {
      Serial.println(F("Fail Safe hit 7000"));
    }
    
    if(failSafeCounter > 10000) {
      Serial.println(F("Fail Safe Hit... Breaking Loop"));
      isGateMoving = false;
    }
    
    /* gatePos = analogRead(analogGatePositionPin); */

    // Get a sampling of the data
    for (int i = 0; i < sampleSize; i++) {
        sampleData[i] = analogRead(analogGatePositionPin);
    }

    gatePos = getMedian(sampleData, sampleSize);

    if(gatePos < pos2) {
      Serial.println(F("HELLA MOVING----"));
      moveGateClosed(speed2);
    } else if (gatePos < pos2) {
      Serial.println(F("HELLA MOVING---------"));
      moveGateClosed(speed3);
    } else if (gatePos < pos3) {
      Serial.println(F("HELLA MOVING---------------"));
      moveGateClosed(speed4);
    } else if (gatePos < pos4) {
      Serial.println(F("HELLA MOVING-----------------------"));
      moveGateClosed(speed5);
    } else if (gatePos < pos5) {
      Serial.println(F("HELLA MOVING-------------------------------"));
      moveGateClosed(speed5);
    } else if (gatePos < pos6) {
      Serial.println(F("HELLA MOVING-----------------------"));
      moveGateClosed(speed4);
    } else if (gatePos < pos7) {
      Serial.println(F("HELLA MOVING---------------"));
      moveGateClosed(speed3);
    } else if (gatePos < pos8) {
      Serial.println(F("HELLA MOVING---------"));
      moveGateClosed(speed2);
    } else if (gatePos < pos9) {
      Serial.println(F("HELLA MOVING----"));
      moveGateClosed(speed2);
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
    isGateMoving = checkActuatorMotion(analogGatePositionPin, signal);
    
    // Check the alarm button on radio remote
    // while the gate is moving to stop it, if necessary.  
    checkForStopButtonInvocation();
 
  } // While Loop
  Serial.println("Past While Loop");
  stopGate();
  
  // Turn off LED now that we've stopped.
  switchLED(isGateMoving);

  XBee.println("closed");

  // After the gate is done closing. Lock it.
  // Make sure the gate is fully closed before locking gate
  Serial.println("WHAT IS CLOSING STATUS POSITION:");
  Serial.println(gatePos);
  if(!GATE_ARM_TEST && gatePos > CHECK_CLOSE_GATE_POS_TO_FIRE_LOCK) {
    lockGate();  
  }
}

/*
  Opens the gate starting out slowly and gradually
  getting to full speed in the middle of the pistons
  reach.  After the center point in gradually slows
  down until it reaches the position set to align 
  with the lock.
  @Param: signal - "open or close"
*/
void openTheGateIncrementally(String signal) {
  
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
      if (FULL_OPEN == false) {
        moveGateOpen(speed2);
      } else {
        moveGateOpen(speed3);
      }
    } else if (gatePos > pos5) {
      if (FULL_OPEN == false) {
        moveGateOpen(speed2);
      } else {
        moveGateOpen(speed3);
      }
    } else if (gatePos > pos4) {
      moveGateOpen(speed3);
    } else if (gatePos > pos3) {
      moveGateOpen(speed3);
    } else if (gatePos > pos2) {
      moveGateOpen(speed2);
    } else if (gatePos > pos1) {
      moveGateOpen(speed2);
      // Debug: Let's print the position now.
      if (oneTime) {
        oneTime = false; 
        Serial.println("Almost Open,  Printing position....");
      }
      Serial.print(gatePos);
    }

      
    //Put this in the last moveGateOpen function
    if (isGateMoving) {
      isGateMoving = checkActuatorMotion(analogGatePositionPin, signal);
    }
    // Stop the gate before full open
    if (FULL_OPEN == false) {
        Serial.println("\n ************* \n Position reached full upen \n ************\n");
        Serial.print("Gate Position: ");
        Serial.println(gatePos);
        Serial.println("***********************");
        if (gatePos > pos5 && gatePos < pos6) {
            Serial.println("\n ************* \n" + String(gatePos, DEC) + "\n ************\n");
            stopGate();
            isGateMoving = false;
        }
    }
    
    // Check the for the alarm button radio button pressed
    // while the gate is moving to stop it, if necessary.  
    checkForStopButtonInvocation();
 
  } // While Loop
  Serial.println("Past While Loop");
  stopGate();
  
  // Turn off LED now that we've stopped.
  switchLED(isGateMoving);

  XBee.println("opened");
  
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
  Params: -> actuatorPositionPin (int) value from actuator of position
  Params: -> signal (String) can either be "open" or "close"
  Returns:-> isSamePos (boolean) has gate moved over last 50 ms.
*/
int checkActuatorMotion(int actuatorPositionPin, String signal) {
  
    int isSamePos = true;
    int isStillMoving = true;
    int curPos, lastPos;
    int sampleSize = 50;
    int sampleData[sampleSize];
    int highDeviation;
    int lowDeviation;

    // Get a sampling of the data
    for (int i = 0; i < sampleSize; i++) {
        sampleData[i] = analogRead(actuatorPositionPin);
    }

    // Get the median of the sample data.  No need to clear
    // the array.  We will just write over the previous values.
    lastPos = getMedian(sampleData, sampleSize);
    
    // Actual positions fluctuate even if motor is not running
    // We should allow for some deviation in the value to check
    // if it has stopped.
    highDeviation = lastPos + MOVEMENT_CHECK_DEVIATION;
    lowDeviation = lastPos - MOVEMENT_CHECK_DEVIATION;
    
    // Let's get the position from before the first time delay.
    /* lastPos = analogRead(actuatorPositionPin); */
    
    for(int j = 0; j < MOVEMENT_CHECK_SENSITIVITY; j++) {
      // We check for isSamePos here because it
      // is true when counter is 0.  If at anytime
      // the gatePos does not equal the previous one
      // the gate is still moving and we don't need
      // to continue.
      if(isSamePos) {
        
        delay(5);
       
        for (int k = 0; k < sampleSize; k++) {
            sampleData[k] = analogRead(actuatorPositionPin);
        }

        curPos = getMedian(sampleData, sampleSize);
        
        Serial.print("Low Deviation: ");
        Serial.print(lowDeviation);
        Serial.print(" High Deviation: ");
        Serial.println(highDeviation);
        // Values seem to fluctuation wildly if the battery is low
        // and the gate is almost open.  We are going to go ahead and
        // say its open if the value is less than 40
        if((lastPos >= lowDeviation && lastPos <= highDeviation) || (curPos < 40 && signal == "open")) {
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
void switchLED(boolean onState) {
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
  roboclaw.BackwardM2(address, 127);
}

/*
  Move the lock actuator piston to the retracted
  position
*/
void retractLockActuator() {
    
  /* digitalWrite(lockDirection, LOW); */
  /* digitalWrite(lockBrake, LOW); */
  /* analogWrite(lockSpeed, speed5); */
  roboclaw.ForwardM2(address, 127);
  
}

/*
  Stop lock actuator
*/
void stopLock() {
  
  /* digitalWrite(lockDirection, LOW); */
  /* digitalWrite(lockBrake, HIGH); */
  /* analogWrite(lockSpeed, 0); */ 
  roboclaw.ForwardM2(address, 0);
  roboclaw.BackwardM2(address, 0);
  
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
       isLockMoving = checkActuatorMotion(analogLockPositionPin, "close");
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
    if (curPos < pos2 || curPos > pos9) {
      Serial.println("Almost fully retracted ");
      isLockMoving = checkActuatorMotion(analogLockPositionPin, "open"); 
    }
  }
  
  stopLock();
  Serial.println("The Lock is stopped");
     
}

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

float getMedianFloat(float *the_array, int size)
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

//void tellZWaveGateStatus(boolean isGateOpen) {
//    if (isGateOpen) {
//        digitalWrite(zwaveSwitchPin, HIGH);
//        Serial.println("Z-Wave Switch Pin is HIGH");
//    } else {
//        digitalWrite(zwaveSwitchPin, LOW);
//        Serial.println("Z-Wave Switch Pin is LOW");
//    }
//
//    Serial.print("Switching Z-Wave to: ");
//    Serial.println(isGateOpen);
//
//    delay(2000);
//}
// Prints the voltages of the battery and the solar panel
// Param inteval - the interval to print the voltinverages at
void printVoltages(int interval) {
  float batteryVoltage = 0.0;
  float solarVoltage = 0.0;
  char battVoltArray[20];
  const char *msg = "Hello Dare";
  float testVar = 4.3564;
  
 if ((millis() - voltageTimestamp) > interval) {
  batteryVoltage = captureVoltage(batteryVoltageInputPin, 50);
  
  voltageTimestamp = millis();
  
  switchLED(true);
  delay(3000);
  switchLED(false);
  
  Serial.print("Battery Voltage: ");
  Serial.println(batteryVoltage);
  
  Serial.println("Formatted Float Value");
  Serial.println(sizeof(batteryVoltage));
  dtostrf(batteryVoltage, sizeof(batteryVoltage), 2, battVoltArray);
  Serial.println(battVoltArray);
  
  XBee.print("Counter: ");
  XBee.println(xBeeCounter);
  xBeeCounter++;
  
  XBee.print("Voltage: ");
  XBee.println(battVoltArray);
  
  //radioHeadDriver.send((uint8_t *)msg, strlen(msg));
  //radioHeadDriver.waitPacketSent();
 
  // Transmit Voltage to Receiver
  // radioHeadDriver.send((uint8_t *)battVoltArray, strlen(battVoltArray));
  // radioHeadDriver.waitPacketSent();
  
  //radioHeadDriver.send((uint8_t *)batteryVoltage, strlen(batteryVoltage));
  //radioHeadDriver.waitPacketSent();
  
 } 
}

// Gather some voltage data to report
// Param inputPin - Pin to get data from
// Param sampleSize - number of voltage samples we should get before getting the median
// Return median voltage from sample Array
float captureVoltage(int inputPin, int sampleSize) {
  float sampleData[sampleSize];
  float oneSample = 0.0;
  
  for (int i = 0; i < sampleSize; i++) {
    sampleData[i] = getVoltage(inputPin);
  }
  
  return getMedianFloat(sampleData, sampleSize);
  
}

// Calculate the voltage from the Voltage Divider
// the input pin is connected to.
float getVoltage(int inputPin) {
  int rawValue = 0;
  float R1 = 1000000.0; //resistance of R1(1M)
  float R2 = 100000.0;  //resistance of R2(100K)
  float vout = 0.0;
  float vin = 0.0;
  
  rawValue = analogRead(inputPin);
  vout = (rawValue * 3.35) / 1024.0;
  vin = vout / (R2/(R1+R2));
  
  // We want to display 0 if the voltage is less than 1
  if (vin < 0.09) {
   vin = 0.0; 
  }
  
  return vin;
}
