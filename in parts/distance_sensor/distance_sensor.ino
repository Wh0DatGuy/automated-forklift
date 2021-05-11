int Echo = 31;
int Trig = 32;
#define Em_Stop 40

int D;

void setup() 
{
  Serial.begin(9600);
  pinMode(Em_Stop,OUTPUT);
  pinMode(Trig,OUTPUT);
  pinMode(Trig+2,OUTPUT);
  pinMode(Trig+4,OUTPUT);
  pinMode(Trig+6,OUTPUT);
  digitalWrite(Trig,LOW);
}

void loop() 
{
  for(int i=0;i<4;i++)
  {
    digitalWrite(Trig,HIGH);
    delayMicroseconds(10);
    digitalWrite(Trig,LOW);
    D = (pulseIn(Echo,HIGH))*0.34/2;
    if(D <= 50)
    {
      Serial.print("holy shet");
    }
    Echo = Echo+2;
    Trig = Trig+2;
    if(Echo > 37)
    {
      Serial.println();
      Echo = 31;
      Trig = 32;
    }
  }
  //delay(100);
}
