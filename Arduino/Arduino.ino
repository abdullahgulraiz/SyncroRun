//_______________I/O_______________
#define motorPin 4
#define sensorPin 2
#define MotorDriverEnablePin 3
#define arduinoTTLModulePowerPin 7

//_______________General Variables_______________
int maximaUser = 0, maximaHardware = 0, sensorValue = 0, hardwareMaximaCount = 0, userMaximaCount = 0, maximaHardwareDelayTimer = 0;
double userMaximaTime = 0, hardwareMaximaTime = 0, userMaximaTime_prev = 0, hardwareMaximaTime_prev = 0, error = 0, motorPwm = 120, motorPwm_prev = 0, time_diff = 0, time_ratio = 1, time_ratio_prev = 1;
int manualUserMaximaTimer = 0; int x = 0; int maximaHardwareSensorTime = 0; bool changeMotorPwm = false; bool discardFirstSerialVal = false;
int motorPwm_lowerLimit = 110, motorPwm_upperLimit = 170;
bool syncedPhase = false;

//_______________Setup_______________
void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  pinMode(sensorPin, INPUT);
  pinMode(motorPin, OUTPUT);
  pinMode(MotorDriverEnablePin, OUTPUT);
  digitalWrite(MotorDriverEnablePin,HIGH);
  pinMode(arduinoTTLModulePowerPin, OUTPUT);
  digitalWrite(arduinoTTLModulePowerPin, HIGH);
}

//_______________Loop_______________
void loop() {
  ResetVariables();
  Step1();
  Step2();
  Step3();
}

//_______________Functions_______________

void ResetVariables() {
  maximaUser = 0; maximaHardware = 0; sensorValue = 0; hardwareMaximaCount = 0; userMaximaCount = 0; maximaHardwareDelayTimer = 0;
  userMaximaTime = 0; hardwareMaximaTime = 0; userMaximaTime_prev = 0; hardwareMaximaTime_prev = 0; error = 0; motorPwm = 120; time_diff = 0; time_ratio = 1;
  manualUserMaximaTimer = 0; x = 0; maximaHardwareSensorTime = 0; changeMotorPwm = false; discardFirstSerialVal = false;
}

void Step1() {
  Serial.println("_____ 1 _____");
  while(1) {
    processSensorValue();
    printValues();
    if (maximaHardware == 1) {
      motorPwm = 0;
      runMotor();
      break;
    }
    runMotor();
  }
}

void Step2() {
  Serial.println("_____ 2 _____");
  while(1) {
    //setMaximaUser();
    getSerial();
    printValues();
    if (maximaUser) {
      if (!discardFirstSerialVal) {
        motorPwm = 150;
        maximaHardware = 0;
        runMotor();
        delay(300);
        userMaximaTime = millis();
        hardwareMaximaTime = userMaximaTime;
        break;
      }
      discardFirstSerialVal = false;
      maximaUser = 0;
    }
    runMotor();
  }
}

void Step3() {
  Serial.println("_____ 3 _____");
  while(1) {
    //setMaximaUser();
    getSerial();
    processSensorValue();
    printValues();
    if (maximaUser) {
      userMaximaTime_prev = userMaximaTime;
      userMaximaTime = millis();
      userMaximaCount++;
      changeMotorPwm = false;
    }
    if (maximaHardware == 1) {
      hardwareMaximaTime_prev = hardwareMaximaTime;
      hardwareMaximaTime = millis();
      hardwareMaximaCount++;
      changeMotorPwm = false;
    }
//    //EXTRA - Eliminate phase difference
    time_diff = hardwareMaximaTime - userMaximaTime;
    //&& time_ratio > 0.80 && time_ratio < 1.20 && time_ratio != 1
    if ((userMaximaCount == hardwareMaximaCount) && abs(time_diff) >= 200 && syncedPhase == false) {
      Serial.println("Starting Step 5");
      Step5();
      Serial.println("Ending Step 5");
      syncedPhase = true;
    }
    if (userMaximaCount == hardwareMaximaCount) {
      if (changeMotorPwm == false) {
        Serial.println("Recalculating Time Ratio");
        time_ratio = (hardwareMaximaTime - hardwareMaximaTime_prev) / (userMaximaTime - userMaximaTime_prev);
        motorPwm *= time_ratio;
        if (motorPwm > motorPwm_upperLimit) { motorPwm = motorPwm_upperLimit; }
        if (motorPwm < motorPwm_lowerLimit) { motorPwm = motorPwm_lowerLimit; }
        changeMotorPwm = true;
        syncedPhase = false;
      }
    }
    if ((abs(userMaximaCount - hardwareMaximaCount) >= 2)) {
      Serial.println("going in step 4");
      Step4();
      Serial.println("exited from step 4");
    }
    runMotor();
  }
}

void Step4() {
  Serial.println("store values");
  hardwareMaximaTime_prev = hardwareMaximaTime;
  userMaximaTime_prev = userMaximaTime;
  Serial.print("Hardware Maxima Time:");
  Serial.print("\t");
  Serial.print(hardwareMaximaTime);
  Serial.print("User Maxima Time:");
  Serial.print("\t");
  Serial.print(userMaximaTime);
  motorPwm_prev = motorPwm;
  time_ratio_prev = time_ratio;
  Serial.println("stop hardware at maxima");
  while(1) {
    processSensorValue();
    printValues();
    if (maximaHardware == 1) {
      motorPwm = 0;
      runMotor();
      hardwareMaximaTime = millis();
      Serial.print("Hardware Maxima Time:");
      Serial.print("\t");
      Serial.println(hardwareMaximaTime);
      hardwareMaximaCount++;
      break;
    }
    runMotor();
  }
  Serial.println("wait for user");
  while(1) {
    getSerial();
    printValues();
    if (maximaUser) {
        userMaximaTime = millis();
        Serial.print("User Maxima Time:");
        Serial.print("\t");
        Serial.println(userMaximaTime);
        userMaximaCount++;
        motorPwm = motorPwm_prev;
        time_ratio = (hardwareMaximaTime - hardwareMaximaTime_prev) / (userMaximaTime - userMaximaTime_prev);
        if (time_ratio > 1.4) { time_ratio = 1.4; }
        motorPwm *= time_ratio;
        Serial.print("Step 4 Calculated values:");
        Serial.print("Time ratio: ");
        Serial.println(time_ratio);
        Serial.print("Motor pwm: ");
        Serial.println(motorPwm);
        if (motorPwm > motorPwm_upperLimit) { motorPwm = motorPwm_upperLimit; }
        if (motorPwm < motorPwm_lowerLimit) { motorPwm = motorPwm_lowerLimit; }
        printValues();
        maximaHardware = 0;
        //hardwareMaximaTime = userMaximaTime;
        maximaUser = 0;
        userMaximaCount = 0; hardwareMaximaCount = 0;
        runMotor();
        delay(300);
        changeMotorPwm = true;
        break;
    }
  }
}

void Step5() {
  Serial.print("Time Difference:");
  Serial.print("\t");
  Serial.println(abs(time_diff));
//  Serial.print("Hardware Maxima Time:");
//  Serial.print("\t");
//  Serial.println(hardwareMaximaTime);
//  Serial.print("User Maxima Time:");
//  Serial.print("\t");
//  Serial.println(userMaximaTime);
  motorPwm_prev = motorPwm;
  time_ratio_prev = time_ratio;
  syncedPhase = true;
  Serial.println("Taking mechanism to extreme");
  while(1) {
    processSensorValue();
    printValues();
    if (maximaHardware == 1) {
      motorPwm = 0;
      runMotor();
      Serial.print("Hardware Maxima Time:");
      Serial.print("\t");
      Serial.println(hardwareMaximaTime);
      hardwareMaximaTime_prev = hardwareMaximaTime;
      hardwareMaximaTime = millis();
      hardwareMaximaCount++;
      break;
    }
    runMotor();
  }
  Serial.println("Waiting for serial value");
  while(1) {
    getSerial();
    printValues();
    if (maximaUser) {
      printValues();
      userMaximaTime_prev = userMaximaTime;
      userMaximaTime = millis();
      hardwareMaximaTime = userMaximaTime;
      Serial.print("User Maxima Time:");
      Serial.print("\t");
      Serial.println(userMaximaTime);
      maximaHardware = 0;
      maximaUser = 0;
      motorPwm = 140;
      runMotor();
      delay(400);
      motorPwm = motorPwm_prev;
      runMotor();
      changeMotorPwm = true;
      userMaximaCount++;
      break;
    }
  }
}

void Step5_alt() {
  Serial.print("Time Difference:");
  Serial.print("\t");
  Serial.println(abs(time_diff));
  Serial.print("Hardware Maxima Time:");
  Serial.print("\t");
  Serial.println(hardwareMaximaTime);
  Serial.print("User Maxima Time:");
  Serial.print("\t");
  Serial.println(userMaximaTime);
  

  int time_hardware = 0, time_user = 0;
  maximaUser = 0; maximaHardware = 0;
  while(1) {
    processSensorValue();
    getSerial();
    if (maximaUser && time_user == 0) {
      time_user = millis();
      Serial.print("Step 5 Time User: ");
      Serial.println(time_user);
    }
    if (maximaHardware == 1 && time_hardware == 0) {
      time_hardware = millis();
      Serial.print("Step 5 Time Hardware: ");
      Serial.println(time_hardware);
    }
    if (time_user != 0 && time_hardware != 0) {
      break;
    }
    runMotor();
  }
  maximaUser = 0; maximaHardware = 0;
  if (abs(time_user - time_hardware) < 700) {
    Serial.println("No need to sync phase");
    syncedPhase = true;
    return;
  }
  Serial.println("Taking hardware to extreme");
  while(1) {
    processSensorValue();
    printValues();
    if (maximaHardware == 1) {
      hardwareMaximaTime_prev = hardwareMaximaTime;
      userMaximaTime_prev = userMaximaTime;
      motorPwm_prev = motorPwm;
      time_ratio_prev = time_ratio;
      motorPwm = 0;
      runMotor();
      Serial.print("Hardware Maxima Time:");
      Serial.print("\t");
      Serial.println(hardwareMaximaTime);
      break;
    }
    runMotor();
  }
  Serial.println("Waiting for serial signal");
  while(1) {
    getSerial();
    printValues();
    if (maximaUser) {
      motorPwm = motorPwm_prev;
      printValues();
      maximaHardware = 0;
      maximaUser = 0;
      runMotor();
      delay(300);
      userMaximaTime = millis();
      hardwareMaximaTime = userMaximaTime;
      changeMotorPwm = true;
      userMaximaCount++;
      syncedPhase = true;
      break;
    }
  }
}

void Step5_alt2() {
    int additional_pwm = (((hardwareMaximaTime - hardwareMaximaTime_prev) - (userMaximaTime - userMaximaTime_prev))/(hardwareMaximaTime - hardwareMaximaTime_prev)) * motorPwm;
    Serial.print("Additional PWM: ");
    Serial.println(additional_pwm);
    motorPwm += additional_pwm;
    time_diff = 0;
}

void processSensorValue() {
  bool sensorVal = !digitalRead(sensorPin); //get sensor value
  if (maximaHardwareSensorTime == 0 && sensorVal == 1) { //if first value for maxima
    maximaHardwareSensorTime = millis(); //store current time, of maxima retrieval
    maximaHardware = 1; //send a single pulse of 1
    return;
  }
  int elapsed_time = (millis() - maximaHardwareSensorTime); //calculate elapsed time since first single pulse of maxima
  if (elapsed_time <= 300) {
    maximaHardware = 0; //if less than 300ms, manually nullify future maxima pulses
  }
  else {
    maximaHardwareSensorTime = 0; //ready to get a new value for maxima
  }
}

void printValues() {
  Serial.print("User Maximas: ");
  Serial.print(userMaximaCount);
  Serial.print("\t  ");
  Serial.print("Hardware Maximas: ");
  Serial.print(hardwareMaximaCount);
  Serial.print("\t  ");
  Serial.print("User Trajectory Time: ");
  Serial.print(userMaximaTime - userMaximaTime_prev);
  Serial.print("\t");
  Serial.print("Hardware Trajectory Time: ");
  Serial.print(hardwareMaximaTime - hardwareMaximaTime_prev);
  Serial.print("\t");
  Serial.print("Time Ratio: ");
  Serial.print(time_ratio);
  Serial.print("\t");
  Serial.print("Motor PWM: ");
  Serial.print(motorPwm);
  Serial.print("\n");
}

void runMotor() {
  analogWrite(motorPin, motorPwm);
}

void getSerial() {
  maximaUser = 0;
  if (Serial1.available()) {
    maximaUser = Serial1.read();
  }
}

