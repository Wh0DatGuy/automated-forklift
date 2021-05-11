#include <Tone.h>

// defines pins numbers
#define stepPinM 3
#define dirPinM1 4
#define dirPinM2 5
#define BUZZER 50
#define dirPinT 52
#define stepPinT 53

long t0 = 2000,t1 = 500;
int HI_LO,BUZZ_ON_OFF,MOTOR_INSTR=5;

Tone tone1;
Tone tone2;

void setup() {
  // Sets the two pins as Outputs
  pinMode(dirPinM1,OUTPUT);
  pinMode(stepPinM,OUTPUT); 
  pinMode(stepPinT,OUTPUT); 
  pinMode(dirPinM2,OUTPUT);
  pinMode(dirPinT,OUTPUT);
  pinMode(BUZZER,OUTPUT);
  
  tone1.begin(stepPinM);
  tone2.begin(stepPinT);
}
void loop() 
{
  
  switch(MOTOR_INSTR)
  {
    case 0/*forward*/: tone1.play(500); digitalWrite(dirPinM2,HIGH); digitalWrite(dirPinM1,LOW);
    break;   
    case 1/*backwards*/: tone1.play(500); digitalWrite(dirPinM2,LOW); digitalWrite(dirPinM1,HIGH);
    break; 
    case 2/*left*/: tone1.play(300); digitalWrite(dirPinM2,LOW); digitalWrite(dirPinM1,LOW);
    break; 
    case 3/*righ*/: tone1.play(300); digitalWrite(dirPinM2,HIGH); digitalWrite(dirPinM1,HIGH); //right up to this point
    break; 
    case 4/*up*/: tone2.play(300); digitalWrite(dirPinT,HIGH); 
    break; 
    case 5/*down*/: tone2.play(300); digitalWrite(dirPinT,LOW); //directions still to check
    break; 
  }
}
