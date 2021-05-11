//libraries
#include <Arduino.h>
#include <IRLibRecv.h>
#include <IRLibDecodeBase.h>
#include <IRLib_P01_NEC.h>
#include <IRLibCombo.h>
//pins
#define stepPinM 22
#define dirPinM1 23
#define dirPinM2 24
#define stepPinT 25
#define dirPinT 26
#define BUZZER 27
#define L_IR_SENSOR 28
#define C_IR_SENSOR 29
#define R_IR_SENSOR 30
#define FrEcho 31 //31,33,35,37 (frontale, destra, retro, sinistra)
#define FrTrig 32 //32,34,36,38
#define ArEcho 33
#define ArTrig 34
#define M_Stop 39
#define Em_Stop 40
#define S2 41
#define S3 42
#define sensorOut_1 43 //box sensor
#define sensorOut_2 44 //floor sensor
#define EndStop 45
#define IrSensor 46
//variables
#define IR_SENSOR_DELAY 200
#define PROX_SENSOR_DELAY 10
long t0 = 0;
bool EnDown;
bool Box = false;
int BoxColor, FloorColor;

//move
void Move(int M_Type, int TrSpeed = 500, int MotSpeed = 1500, int UpDwSpeed = 12000) {
  bool BUZZERVal = false;
  switch (M_Type) {
  case 1 /*forward*/:
    digitalWrite(dirPinM2, HIGH);
    digitalWrite(dirPinM1, LOW);
    tone(stepPinM, MotSpeed);
    break;

  case 2 /*backwards*/:
    digitalWrite(dirPinM2, LOW);
    digitalWrite(dirPinM1, HIGH);
    BUZZERVal = true;
    tone(stepPinM, MotSpeed);
    break;

  case 3 /*left*/:
    digitalWrite(dirPinM2, HIGH);
    digitalWrite(dirPinM1, HIGH);
    BUZZERVal = true;
    tone(stepPinM, TrSpeed);
    break;

  case 4 /*right*/:
    digitalWrite(dirPinM2, LOW);
    digitalWrite(dirPinM1, LOW);
    BUZZERVal = true;
    tone(stepPinM, TrSpeed);
    break;

  case 5 /*up*/:
    digitalWrite(dirPinT, LOW);
    noTone(stepPinM);
    tone(stepPinT, UpDwSpeed);
    break;

  case 6 /*down*/:
    digitalWrite(dirPinT, HIGH);
    noTone(stepPinM);
    tone(stepPinT, UpDwSpeed);
    break;

  case 7 /*stop*/:
    noTone(stepPinT);
    noTone(stepPinM);
    break;
  }

  digitalWrite(BUZZER, BUZZERVal);
}
//detect color
int Color_set(int S2state, int S3state, int SensorOut) { //read sensor function
  int rVal;
  digitalWrite(S2, S2state);
  digitalWrite(S3, S3state);
  rVal = (pulseIn(SensorOut, LOW) + pulseIn(SensorOut, LOW) + pulseIn(SensorOut, LOW))/3;
  return rVal;
}
int Color(int S_Type) {
  int ColorNum;
  int R, G, B;
  //char buffer[10];

  switch (S_Type) { //selecting sensor
  case 1 /*box sensor*/:
    R = Color_set(LOW, LOW, sensorOut_1);
    G = Color_set(HIGH, HIGH, sensorOut_1);
    B = Color_set(LOW, HIGH, sensorOut_1);
    //sprintf(buffer, "%d  %d  %d", R, G, B);
    //Serial.println(buffer);
    //Serial.flush();
      if ((R <= 39) && (G <= 38) && (B <= 35)) //green
        ColorNum = 2;
      else if ((R <= 54) && (G <= 52) && (B <= 31)) //blue
        ColorNum = 3;
    break;

  case 2 /*floor sensor*/:
    R = Color_set(LOW, LOW, sensorOut_2);
    G = Color_set(HIGH, HIGH, sensorOut_2);
    B = Color_set(LOW, HIGH, sensorOut_2);
    //sprintf(buffer, "%d  %d  %d", R, G, B);
    //Serial.println(buffer);
    //Serial.flush();
      if ((R <= 19) && (G <= 23) && (B <= 26)) //yellow
        ColorNum = 1;
      else if ((R <= 41) && (G <= 32) && (B <= 32)) //green
        ColorNum = 2;
      else if ((R <= 54) && (G <= 46) && (B <= 28)) //blue
        ColorNum = 3;
    break;
  }
  return ColorNum;
}
//emergency stop
void EmStop() {
  digitalWrite(Em_Stop, HIGH);
  Move(7);
  delay(500);
  digitalWrite(BUZZER, HIGH);
  digitalWrite(M_Stop, HIGH);
  while (true) { }
}
//follow line
bool FwLine() {
  bool OLine = false;
  if (digitalRead(L_IR_SENSOR) && digitalRead(R_IR_SENSOR)) {
    Move(7);
    OLine = true;
  } else if (digitalRead(L_IR_SENSOR)) {
    Move(3, 1500);
    t0 = millis();
  } else if (digitalRead(R_IR_SENSOR)) {
    Move(4, 1500);
    t0 = millis();
  } else {
    Move(1);
    if (!digitalRead(C_IR_SENSOR)) {
      if ((millis() - t0) >= 900) {
        //Serial.println("line not detected");
        EmStop();
      }
    } else {
      t0 = millis();
    }
  }
  return OLine;
}
//auto proximity sensors
unsigned int ReadPxSensor(int Trig, int Echo) {
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
  return ((pulseIn(Echo, HIGH)) * 0.34 / 2);
}
void AProxSensor(int Ap_Type, bool EnFrontSn = true) {
  static long t1;
  switch (Ap_Type) {
  case 1 /*emergency stop*/:
    if ((millis() - t1) >= 500) {
      if (EnFrontSn) {
        if (ReadPxSensor(FrTrig, FrEcho) < 120) {
          //Serial.print("proximity sensor error front");
          EmStop();
        }
      }
      if (ReadPxSensor(ArTrig, ArEcho) < 100) {
        //Serial.print("proximity sensor error sides");
        EmStop();
      }
      t1 = millis();
    }
    break;

  case 2 /*box aproaching at pick-up*/:
    while (ReadPxSensor(FrTrig, FrEcho) > 25) {
      FwLine();
      AProxSensor(1, false);
      delay(PROX_SENSOR_DELAY);
    }
    break;

  case 3 /*box clearance at departure*/:
    Move(2);
    while (ReadPxSensor(FrTrig, FrEcho) < 170) {
      AProxSensor(1, false);
      delay(PROX_SENSOR_DELAY);
    }
    break;
  }
}
//auto move
void AMove(int AM_Type) {
  switch (AM_Type) {
  case 0 /*auto up*/:
    Move(5);
    while (!digitalRead(EndStop)) { }
    Move(7);
    EnDown = true;
    break;

  case 1 /*auto down*/:
    if (EnDown) {
      Move(6);
      delay(4300);
      Move(7);
      EnDown = false;
    }
    break;

  case 2 /*line jump*/:
    Move(1);
    delay(400);
    t0 = millis();
    break;

  case 3 /*180*/:
  case 4 /*90 left*/:
    Move(3);
    while (digitalRead(L_IR_SENSOR)) {
      AProxSensor(1);
    }
    delay(IR_SENSOR_DELAY);
    while (!digitalRead(L_IR_SENSOR)) {
      AProxSensor(1);
    }
    delay(IR_SENSOR_DELAY);
    while (digitalRead(L_IR_SENSOR)) {
      AProxSensor(1);
    }
    delay(IR_SENSOR_DELAY);
    t0 = millis();
    break;

  case 5 /*90 right*/:
    Move(4);
    while (digitalRead(R_IR_SENSOR)) {
      AProxSensor(1);
    }
    delay(IR_SENSOR_DELAY);
    while (!digitalRead(R_IR_SENSOR)) {
      AProxSensor(1);
    }
    delay(IR_SENSOR_DELAY);
    while (digitalRead(R_IR_SENSOR)) {
      AProxSensor(1);
    }
    delay(IR_SENSOR_DELAY);
    t0 = millis();
    break;
  }
}
//ir remote
IRrecv irrecv(IrSensor);
IRdecode Signal;

void IrRemote() {
  irrecv.enableIRIn();
  bool IrActive = true;

  while (IrActive) {
    AProxSensor(1);
    if (irrecv.getResults()) {
      Signal.decode();
      //Serial.println(Signal.value);
      switch (Signal.value) {
      case 16718055 /*forward*/:
        Move(1);
        break;

      case 16716015 /*left*/:
        Move(3);
        break;

      case 16734885 /*right*/:
        Move(4);
        break;

      case 16730805 /*back*/:
        Move(2);
        break;

      case 16738455 /*pick up*/:
        while (ReadPxSensor(FrTrig, FrEcho) > 25) {
          Move(1);
          delay(PROX_SENSOR_DELAY);
        }
        AMove(0);
        break;

      case 16756815 /*put down*/:
        AMove(1);
        while (ReadPxSensor(FrTrig, FrEcho) < 170) {
          Move(2);
          delay(PROX_SENSOR_DELAY);
        }
        Move(7);
        break;

      case 16750695 /*stop*/:
        Move(7);
        break;

      case 16726215 /*OK*/:
        IrActive = false;
        break;
      }
      irrecv.enableIRIn();
    }
  }
  irrecv.disableIRIn();
  Move(7);
}
//main program
void setup() {
  pinMode(stepPinM, OUTPUT);
  pinMode(dirPinM1, OUTPUT);
  pinMode(dirPinM2, OUTPUT);
  pinMode(stepPinT, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(dirPinT, OUTPUT);
  pinMode(M_Stop, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(Em_Stop, OUTPUT);
  pinMode(FrTrig, OUTPUT);
  pinMode(ArTrig, OUTPUT);
  digitalWrite(FrTrig, LOW);
  digitalWrite(ArTrig, LOW);
  AMove(0);
  AMove(1);
  //Serial.begin(9600);
  IrRemote();
}

void loop() {
  AProxSensor(1);
  if (FwLine()) {
    //while (true){Color(2);Move(7);} //decomment for floor color calibration
    FloorColor = Color(2);
    if (FloorColor == 1) { //red
      if (!Box) { //check if already have box
        AMove(2);       //jump line
        AProxSensor(2); //approach box
        BoxColor = Color(1);
        AMove(0); //lift box
        //while (true) {Color(1);}//decomment for box color calibration
        AMove(3);   //turn around
        Box = true; //signal box picked up
      } else {
        AMove(3); //turn around
      }
    } else if ((BoxColor == FloorColor) && Box) { //green or blue
      AMove(5); //turn right
      while (!FwLine()) {
        AProxSensor(1);
      }         //wait for stop
      AMove(1); //lower box
      AProxSensor(3); //leave deposit location
      AMove(3);       //turn around
      while (!FwLine()) {
        AProxSensor(1);
      }         //wait for stop
      AMove(2); //skip line
      delay(800);
      AMove(4);    //turn left to pick up next box
      Box = false; //signal box put down
    } else if (FloorColor == 3) { //blue
      AMove(3); //turn around end of the trace
    } else {
      AMove(2); //not right place, go on
    }
  }
}
