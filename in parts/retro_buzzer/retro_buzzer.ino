#define BUZZER 3

long T;

void setup() 
{
  pinMode(BUZZER,OUTPUT);
}

void loop() 
{
  if(millis()-T >= 1000)
  {
    tone(BUZZER,4000,500);
    T=millis();
  }
}
