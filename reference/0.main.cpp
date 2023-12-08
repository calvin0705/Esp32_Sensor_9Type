#include <Arduino.h>

int result = 0;

int myFunction(int x, int y) {
  return x + y;
}

void setup() {
  Serial.begin(115200);
}

void loop() {
  result = myFunction(2, 3);
  Serial.printf("myFunction ==>> %d \n", result);
  delay(2000);
}

