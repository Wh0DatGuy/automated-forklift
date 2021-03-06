#include <IRLibRecv.h>
#include <IRLibDecodeBase.h>
#include <IRLib_P01_NEC.h>
#include <IRLibCombo.h> 

#define IrSensor 46

long IrActive;

IRrecv irrecv(IrSensor);
IRdecode Signal;

void setup() 
{
  Serial.begin(9600);
  irrecv.enableIRIn();
}

void loop() 
{
  if(irrecv.getResults())
  {
    Signal.decode();
    Serial.println(Signal.value);
    irrecv.enableIRIn(); // Receive the next value
    IrActive = Signal.value;
    //Serial.println(IrActive);
    while(IrActive ==               16726215)//OK activates remote control
    {
      if(irrecv.getResults())
      {
        Signal.decode();
        irrecv.enableIRIn(); // Receive the next value
        switch (Signal.value) 
        {
          case 16718055/*forward*/:
            Serial.println("forward");
          break;
          case 16716015/*left*/:
            Serial.println("left");
          break;
          case 16734885/*right*/:
            Serial.println("right");
          break;
          case 16730805/*back*/:
            Serial.println("back");
          break;
          case 16738455/*up*/:
            Serial.println("up");
          break;
          case 16756815/*down*/:
            Serial.println("down");
          break;
          case 16750695/*stop*/:
            Serial.println("stop");
          break;
          case 16726215/*OK*/:
            Serial.println("OK");
            IrActive = 0;
          break;
        }
      }
    }
  }
}
