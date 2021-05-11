//if both sensors are 0 then check the color
#define LEFT_IR_SENSOR 3
#define RIGHT_IR_SENSOR 2

void setup() 
{
  Serial.begin(9600);
}

void loop() 
{
  if((digitalRead(LEFT_IR_SENSOR) == 1) && (digitalRead(RIGHT_IR_SENSOR) == 1))
  {
    Serial.println("checking color");
  }
  else if(digitalRead(LEFT_IR_SENSOR) == 1)
  {
    Serial.println("going left");
  }
  else if(digitalRead(RIGHT_IR_SENSOR) == 1)
  {
    Serial.println("going right");
  }
}
