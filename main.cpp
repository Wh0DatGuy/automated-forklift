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
bool StandByf = false, BtEn = true, Going = false, FSEn = true, PickBox = false, PutBox = false, TurnLeft = false, TurnRight = false, ReturnLine = false;
int BoxPos[3], CurBoxPos[3] = { 0, 0, 0 }, CurPos = 0;
//lib
Tone Step[3];
//move
void Move(int M_Type, int TrSpeed = 500) {
  int UpDwSpeed = 12000, MotSpeed = 1800;
  bool BUZZERVal = false;
  StandByf = false;
  switch (M_Type) {
  case 1 /*forward*/:
  case 2 /*backwards*/:
  if(M_Type == 1) {
    digitalWrite(dirPinM2, HIGH);
    digitalWrite(dirPinM1, LOW);
  } else {
    digitalWrite(dirPinM2, LOW);
    digitalWrite(dirPinM1, HIGH);
    BUZZERVal = true;
  }
    Step[2].stop();
    Step[0].play(MotSpeed);
    Step[1].play(MotSpeed);
    break;

  case 3 /*sharp left*/:
  case 4 /*sharp right*/:
  if(M_Type == 3) {
    digitalWrite(dirPinM2, HIGH);
    digitalWrite(dirPinM1, HIGH);
  } else {
    digitalWrite(dirPinM2, LOW);
    digitalWrite(dirPinM1, LOW);
  }
    BUZZERVal = true;
    Step[2].stop();
    Step[0].play(TrSpeed);
    Step[1].play(TrSpeed);
    break;

  case 5 /*up*/:
  case 6 /*down*/:
  if(M_Type == 5) {
    digitalWrite(dirPinT, LOW);
  } else {
    digitalWrite(dirPinT, HIGH);
  }
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
  case 9 /*slow right*/:
    digitalWrite(dirPinM2, HIGH);
    digitalWrite(dirPinM1, LOW);
    Step[2].stop();
    if(M_Type == 8) {
    Step[0].play(MotSpeed / 2);
    Step[1].play(MotSpeed);
  } else {
    Step[0].play(MotSpeed);
    Step[1].play(MotSpeed / 2);
  }
    break;
  }
  digitalWrite(BUZZER, BUZZERVal);
}
//emergency stop
void EmStop() {
  digitalWrite(Em_Stop, HIGH);
  Move(7);
  t2 = millis();
  BtEn = true;
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
//follow line
bool FwLine() {
  bool OLine = false;
  if (digitalRead(L_IR_SENSOR) && digitalRead(R_IR_SENSOR)) {
    delay(10);
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
  return Dist[Sel];
}
//auto proximity sensors
bool AProxSensor(int Ap_Type, bool EnFrontSn = true) {
  bool BoxReady = false;
  switch (Ap_Type) {
  case 1 /*emergency stop*/:
    if ((ReadPxSensor(FrTrig, FrEcho) < 150) && (EnFrontSn)) {
      EmStop();
    }
    if ((ReadPxSensor(BrTrig, BrEcho, 1) < 100)) {
      EmStop();
    }
    break;

  case 2 /*box aproaching at pick-up*/:
    if (ReadPxSensor(FrTrig, FrEcho) < 25) {
      BoxReady = true;
    }
    break;

  case 3 /*box clearance at departure*/:
    if (ReadPxSensor(FrTrig, FrEcho) > 170) {
      BoxReady = true;
    }
    break;
  }
  return BoxReady;
}
//edge function
bool IfUpOrDwSn(byte Pin2Read, bool Mode) {//Mode = 0 (up), 1 (down)
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
      while ((millis() - t4) <= 4300) { }
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
//bluetooth
void(*Riavvia)(void) = 0;
void bluetooth() {
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

    case 'u' /*up*/:
      StandByf = false;
      StandBy();
      AMove(0);
      break;

    case 'd' /*down*/:
      StandByf = false;
      StandBy();
      AMove(1);
      break;

    case 'O' /*OK*/:
      digitalWrite(M_Stop, LOW);
      Move(7);
      digitalWrite(Em_Stop, LOW);
      t0 = millis();
      BtEn = false;
      break;

    case 'C':
      Riavvia();
      break;
    }
  }
  AProxSensor(1, !OverWrite);
  StandBy();
}
//barcode scanner
bool SerialBarcode() {
  byte AcceptScan[7] = { 2, 0, 0, 1, 0, 51, 49 };
  byte StartScan[9] = { 0x7E, 0x00, 0x08, 0x01, 0x00, 0x02, 0x01, 0xAB, 0xCD };
  static int i = 0;
  static bool EnRecvScan, fAcceptScan = true;
  bool ScanFin = false;
  while (!ScanFin) {
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
  }
  return ScanFin;
}
//main program
void setup() {
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
  digitalWrite(M_Stop, LOW);
  AMove(0);//reset fork position
  AMove(1);
  t0 = millis();
}

void loop() {
  if (BtEn) {
    bluetooth();
  } else {
    if (!(TurnLeft || TurnRight || PutBox || ReturnLine) && FwLine()) {
      if (Going) {
        if (BoxPos[CurPos] == CurBoxPos[CurPos]) {
          CurPos++;
          if (CurPos < 3) {
            TurnRight = true;
          }
        } else {
          CurBoxPos[CurPos] ++;
          AMove(2);
        }
        if (CurPos == 3) {
          FSEn = false;
          PutBox = true;
        }
      } else {//add delay to line jump during return
        if ((CurPos == 0) && (CurBoxPos[0] == 0)) {
          AMove(2);
          FSEn = false;
          PickBox = true;
        } else {
          if (CurBoxPos[CurPos] == 0) {
            CurPos--;
            if (CurPos != 0) {
             ReturnLine = true;
            }
          } else {
            CurBoxPos[CurPos] --;
            AMove(2);
          }
        }
      }
    }
    if (PickBox && AProxSensor(2)) {
      Move(7);
      if (SerialBarcode()) {
        AMove(0);
        TurnRight = true;
        FSEn = true;
        Going = true;
        PickBox = false;
      }
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
    } else if (PutBox) {
      AMove(1);
      Move(2);
      if (AProxSensor(3)) {
        TurnLeft = true;
        FSEn = true;
        Going = false;
        PutBox = false;
      }
    } else if (ReturnLine) {
      Move(1);
      if(IfUpOrDwSn(C_IR_SENSOR, 0)) {
        TurnLeft = true;
        ReturnLine = false;
      }
    }
    AProxSensor(1, FSEn);
  }
}
