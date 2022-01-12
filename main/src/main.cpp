#include <Arduino.h>
#include <Tone.h>
//pins
#define stepPinM1 22
#define stepPinM2 35
#define stepPinT 25
#define dirPinM1 23
#define dirPinM2 24
#define dirPinT 26
#define BUZZER 27
#define L_IR_SENSOR 28
#define C_IR_SENSOR 29
#define R_IR_SENSOR 30
#define FrEcho 31 //frontale
#define FrTrig 32 
#define BrEcho 33 //retro
#define BrTrig 34
#define M_Stop 39
#define Em_Stop 40
#define EndStop 45
//variables
#define IR_SENSOR_DELAY 200
#define PROX_SENSOR_DELAY 10
long t0 = 0, t2;
bool StandByf = false, BtEn = true, Going = false, FSEn = true, PickBox = false, PutBox = false, TurnLeft = false, TurnRight = false;
int BoxPos[2], CurBoxPos[2] = { 0, 0 }, CurPos = 0;
//lib
Tone Step[3];
//move
void Move(int M_Type, int TrSpeed = 31) {
  int UpDwSpeed = 750, MotSpeed = 100;
  bool BUZZERVal = false;
  StandByf = false;
  switch (M_Type) {
  case 1 /*forward*/:
    digitalWrite(dirPinM2, HIGH);
    digitalWrite(dirPinM1, LOW);
    Step[2].stop();
    Step[0].play(MotSpeed);
    Step[1].play(MotSpeed);
    break;

  case 2 /*backwards*/:
    digitalWrite(dirPinM2, LOW);
    digitalWrite(dirPinM1, HIGH);
    BUZZERVal = true;
    Step[2].stop();
    Step[0].play(MotSpeed);
    Step[1].play(MotSpeed);
    break;

  case 3 /*sharp left*/:
    digitalWrite(dirPinM2, HIGH);
    digitalWrite(dirPinM1, HIGH);
    BUZZERVal = true;
    Step[2].stop();
    Step[0].play(TrSpeed);
    Step[1].play(TrSpeed);
    break;

  case 4 /*sharp right*/:
    digitalWrite(dirPinM2, LOW);
    digitalWrite(dirPinM1, LOW);
    BUZZERVal = true;
    Step[2].stop();
    Step[0].play(TrSpeed);
    Step[1].play(TrSpeed);
    break;

  case 5 /*up*/:
    digitalWrite(dirPinT, LOW);
    Step[0].stop();
    Step[1].stop();
    Step[2].play(UpDwSpeed);
    break;

  case 6 /*down*/:
    digitalWrite(dirPinT, HIGH);
    Step[0].stop();
    Step[1].stop();
    Step[2].play(UpDwSpeed);
    break;

  case 7 /*stop*/:
    Step[0].stop();
    Step[1].stop();
    Step[2].stop();
    StandByf = true;
    break;

  case 8 /*slow left*/:
    digitalWrite(dirPinM2, HIGH);
    digitalWrite(dirPinM1, LOW);
    Step[2].stop();
    Step[0].play(MotSpeed / 2);
    Step[1].play(MotSpeed);
    break;

  case 9 /*slow right*/:
    digitalWrite(dirPinM2, HIGH);
    digitalWrite(dirPinM1, LOW);
    Step[2].stop();
    Step[0].play(MotSpeed);
    Step[1].play(MotSpeed / 2);
    break;
  }
  digitalWrite(BUZZER, BUZZERVal);
}
//emergency stop
bool bluetooth();
void EmStop() {
  digitalWrite(Em_Stop, HIGH);
  Move(7);
  t2 = millis();
  BtEn = true;
}
//follow line
bool FwLine() {
  bool OLine = false;
  if (digitalRead(L_IR_SENSOR) && digitalRead(R_IR_SENSOR)) {
    Move(7);
    OLine = true;
  } else if (digitalRead(L_IR_SENSOR)) {
    Move(8);
    t0 = millis();
  } else if (digitalRead(R_IR_SENSOR)) {
    Move(9);
    t0 = millis();
  } else {
    Move(1);
    if (!digitalRead(C_IR_SENSOR)) {
      if ((millis() - t0) >= 500) {
        //Serial.println("line not detected");
        EmStop();
      }
    } else {
      t0 = millis();
    }
  }
  return OLine;
}
//read proximity sensors
int ReadPxSensor(int Trig, int Echo, int Sel = 0) {
  static int Dist[2], oSel;
  static long t1;
  if ((millis() - t1) >= 60 || oSel != Sel) {
    digitalWrite(Trig, LOW);
    delayMicroseconds(2);
    digitalWrite(Trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(Trig, LOW);
    Dist[Sel] = (pulseIn(Echo, HIGH) / 2) * 0.343;
    oSel = Sel;
    t1 = millis();
  }
  //Serial.println(Dist[Sel]);
  return Dist[Sel];
}
//auto proximity sensors
bool AProxSensor(int Ap_Type, bool EnFrontSn = true) {
  static bool BoxReady = false;
  switch (Ap_Type) {
  case 1 /*emergency stop*/:
    if ((ReadPxSensor(FrTrig, FrEcho) < 150) && (EnFrontSn)) {
      //Serial.print("proximity sensor error front");
      EmStop();
    }
    if ((ReadPxSensor(BrTrig, BrEcho, 1) < 100)) {
      //Serial.print("proximity sensor error back");
      EmStop();
    }
    break;

  case 2 /*box aproaching at pick-up*/:
    if (ReadPxSensor(FrTrig, FrEcho) < 28) {//remember to disable front emergency sensor
      BoxReady = true;
    }
    break;

  case 3 /*box clearance at departure*/:
    if (ReadPxSensor(FrTrig, FrEcho) > 170) {//remember to disale front emergency sensor
      BoxReady = true;
    }
    break;
  }
  return BoxReady;
}
//edge function
bool IfUpOrDwSn(byte Pin2Read, bool Mode) {
  static bool fState = false;
  bool ReturnVal = false;
  bool bn = digitalRead(Pin2Read);
  if (!Mode) {
    bn = !bn;
  }
  if (!bn) {
    fState = true;
  } else if (fState && bn) {
    ReturnVal = true;
    fState = false;
  }
  return ReturnVal;
}
//auto move
bool AMove(int AM_Type, unsigned int DelayVal = 200) {
  static int IfCount;
  bool fTurn = false;
  static long t3, t4;
  static bool EnDown;
  switch (AM_Type) {
  case 0 /*auto up*/:
    Move(5);
    while (!digitalRead(EndStop)) { }
    Move(7);
    EnDown = true;
    break;

  case 1 /*auto down*/:
    if (EnDown) {
      t4 = millis();
      Move(6);
      while ((millis() - t4) <= 4300) {
        AProxSensor(1);
      }
      Move(7);
      EnDown = false;
    }
    break;

  case 2 /*line jump*/:
    t3 = millis();
    while ((millis() - t3) <= DelayVal) {
      Move(1);
      AProxSensor(1);
    }
    t0 = millis();
    break;

  case 4 /*90 left*/:
    if (IfUpOrDwSn(L_IR_SENSOR, 0) && (IfCount == 0)) {
      IfCount = 1;
    } else if (IfUpOrDwSn(L_IR_SENSOR, 1) && (IfCount == 1)) {
      IfCount = 2;
    }
    t0 = millis();
    break;

  case 5 /*90 right*/:
    if (IfUpOrDwSn(R_IR_SENSOR, 0) && (IfCount == 0)) {
      IfCount = 1;
    } else if (IfUpOrDwSn(R_IR_SENSOR, 1) && (IfCount == 1)) {
      IfCount = 2;
    }
    t0 = millis();
    break;
  }
  if (IfCount == 2) {
    IfCount = 0;
    fTurn = true;
  }
  return fTurn;
}
//turn off motors
void StandBy() {
  if (StandByf) {
    if (millis() - t2 >= 5000) {
      digitalWrite(M_Stop, HIGH);
      t2 = millis();
    }
  } else {
    t2 = millis();
    digitalWrite(M_Stop, LOW);
  }
}
//bluetooth
bool bluetooth() {
  static bool OverWrite = false;
  while (Serial3.available()) {
    switch (Serial3.read()) {
    case 'F' /*forward*/:
      Move(1);
      break;

    case 'L' /*left*/:
      Move(3);
      break;

    case 'A' /*slight left*/:
      Move(8);
      break;

    case 'R' /*right*/:
      Move(4);
      break;

    case 'X' /*slight right*/:
      Move(9);
      break;

    case 'B' /*back*/:
      Move(2);
      break;

    case 'S' /*stop*/:
      Move(7);
      break;

    case 'U' /*dist off*/:
      OverWrite = true;
      break;

    case 'V' /*dist on*/:
      OverWrite = false;
      break;

    case 'O' /*OK*/:
      digitalWrite(M_Stop, LOW);
      Move(7);
      digitalWrite(Em_Stop, LOW);
      t0 = millis();
      BtEn = false;
      break;
    }
  }
  StandBy();
  return OverWrite;
}
//barcode scanner
bool SerialBarcode() {
  byte AcceptScan[7] = { 2,0,0,1,0,51,49 };
  byte StartScan[9] = { 0x7E,0x00,0x08,0x01,0x00,0x02,0x01,0xAB,0xCD };
  static int i = 0;
  static bool EnRecvScan, fAcceptScan = true;
  bool ScanFin = false;
  if (EnRecvScan) {
    while (Serial2.available()) {
      switch (Serial2.read()) {
      case 's':
        BoxPos[0] = Serial2.parseInt();
        break;
      case 'p':
        BoxPos[1] = Serial2.parseInt();
        break;
      case 'f':
        i = 0;
        EnRecvScan = false;
        fAcceptScan = true;
        ScanFin = true;
        break;
      }
    }
  } else if (Serial2.available()) {
    byte RecvScan = Serial2.read();
    if ((RecvScan == AcceptScan[i]) && fAcceptScan) {
      fAcceptScan = true;
      i++;
    } else {
      fAcceptScan = false;
    }
    if (fAcceptScan && (i == 6)) {
      EnRecvScan = true;
    }
  } else if (i == 0) {
    Serial2.write(StartScan, sizeof(StartScan));
  }
  return ScanFin;
}
//main program
void setup() {
  //Serial.begin(9600);
  Serial3.begin(9600);
  Serial2.begin(9600);
  pinMode(dirPinM1, OUTPUT);
  pinMode(dirPinM2, OUTPUT);
  pinMode(dirPinT, OUTPUT);
  Step[0].begin(stepPinM1);
  Step[1].begin(stepPinM2);
  Step[2].begin(stepPinT);
  pinMode(BUZZER, OUTPUT);
  pinMode(M_Stop, OUTPUT);
  pinMode(Em_Stop, OUTPUT);
  pinMode(FrTrig, OUTPUT);
  pinMode(BrTrig, OUTPUT);
  digitalWrite(FrTrig, LOW);
  digitalWrite(BrTrig, LOW);
  AMove(0);//reset fork position
  AMove(1);
  t0 = millis();
}

void loop() {
  if (BtEn) {
    if (!bluetooth()) {
      AProxSensor(1, 1);
    }
  } else {
    if ((!(TurnLeft || TurnRight)) && FwLine()) {
      if ((!Going) && (CurBoxPos[0] == 0)) {
        AMove(2);
        FSEn = false;
        PickBox = true;
      }
      if (Going && (CurPos == 2)) {
        FSEn = false;
        PutBox = true;
      }
      if (Going) {
        CurBoxPos[CurPos] ++;
        AMove(2);
      } else {
        CurBoxPos[CurPos] --;
        AMove(2);
      }
      if (BoxPos[CurPos] == CurBoxPos[CurPos]) {
        if (Going) {
          CurPos++;
          TurnRight = true;
        } else {
          CurPos--;
          TurnLeft = true;
        }
      }
    }
    if (PickBox && AProxSensor(2)) {
      Move(7);
      //if (SerialBarcode()) {
      AMove(0);
      TurnRight = true;
      FSEn = true;
      Going = true;
      PickBox = false;
      //}
    } else if (TurnLeft) {
      Move(3);
      if (AMove(4)) {
        TurnLeft = false;
      }
    } else if (TurnRight) {
      Move(4);
      if (AMove(5)) {
        TurnRight = false;
      }
    } else if (PutBox) {//to check
      AMove(1);
      Move(2);
      if (AProxSensor(3)) {
        TurnLeft = true;
        FSEn = true;
        Going = false;
        PutBox = false;
      }
    }
    AProxSensor(1, FSEn);
  }
}
