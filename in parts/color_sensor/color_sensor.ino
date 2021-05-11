void setup() {Serial.begin(9600);}

void loop() {Color(1);}

#define S2 40
#define S3 41
#define sensorOut_1 43
#define sensorOut_2 44
  
int Color(int S_Type)
{
  int SensorOut;
  int R_Frequency;
  int G_Frequency;
  int B_Frequency;
  
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  
  switch (S_Type)//selecting sensor
  {
    case 0/*box sensor*/: SensorOut = sensorOut_1;
    break;
    case 1/*pavement sensor*/: SensorOut = sensorOut_2;
    break;
  }
  // Setting red
  digitalWrite(S2,LOW);
  digitalWrite(S3,LOW);
  // Reading frequency
  R_Frequency = pulseIn(SensorOut, LOW);
  Serial.print(R_Frequency);
  Serial.print("  ");
  // Setting Green
  digitalWrite(S2,HIGH);
  digitalWrite(S3,HIGH);
  // Reading frequency
  G_Frequency = pulseIn(SensorOut, LOW);
  Serial.print(G_Frequency);
  Serial.print("  ");
  // Setting Blue
  digitalWrite(S2,LOW);
  digitalWrite(S3,HIGH);
  // Reading frequency
  B_Frequency = pulseIn(SensorOut, LOW);
  Serial.println(B_Frequency);
  delay(200);
  
  switch(S_Type)
  {
    case 0:
      if ((R_Frequency <= 28) && (G_Frequency <= 38) && (B_Frequency <= 38))//yellow 
      {
        //return 0;
        Serial.println("yellow");
      }
      else if ((R_Frequency <= 37) && (G_Frequency <= 49) && (B_Frequency <= 49))//green
      {
        //return 2;
        Serial.println("green");
      } 
      else if ((R_Frequency <= 36) && (G_Frequency <= 66) && (B_Frequency <= 66))//blue
      {
        //return 3;
        Serial.println("blue");
      }
      else if ((R_Frequency <= 30) && (G_Frequency <= 87) && (B_Frequency <= 87))//red
      {
        //return 1;
        Serial.println("red");
      } 
    break;
    case 1:
      if ((R_Frequency <= 8) && (G_Frequency <= 10) && (B_Frequency <= 10))//yellow 
      {
        //return 0;
        Serial.println("yellow");
      }
      else if ((R_Frequency <= 15) && (G_Frequency <= 15) && (B_Frequency <= 15))//green
      {
        //return 1;
        Serial.println("green");
      } 
      else if ((R_Frequency <= 16) && (G_Frequency <= 20) && (B_Frequency <= 20))//blue
      {
        //return 3;
        Serial.println("blue");
      }
      else if ((R_Frequency <= 19) && (G_Frequency <= 23) && (B_Frequency <= 23))//red
      {
        //return 2;
        Serial.println("red");
      } 
    break;
  }
}
