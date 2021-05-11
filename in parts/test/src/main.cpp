#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
}

void loop() {
  digitalWrite(6, HIGH);
  delayMicroseconds(100);
  digitalWrite(6, LOW);
  digitalWrite(7, HIGH);
  delayMicroseconds(100);
  digitalWrite(6, HIGH);
  delayMicroseconds(100);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  delayMicroseconds(100);
}