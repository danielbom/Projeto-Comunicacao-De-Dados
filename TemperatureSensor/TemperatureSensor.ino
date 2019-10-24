#include <SPI.h>
#include <time.h>

int temperature = 20;

float randomize() {
  return rand() / (float) RAND_MAX;
}

void updateTemperature() {
  bool signal = randomize() > 0.5;
  bool up = randomize() > 0.5;
  if (signal) {
    if (up) {
      Serial.println("LOG: Subindo a temperatura.");
      temperature += 1;
    }
  } else {
    if (up) {
      Serial.println("LOG: Diminuindo a temperatura.");
      temperature -= 1;
    }
  }
  temperature = temperature > 30 ? 30 : temperature;
  temperature = temperature < 16 ? 16 : temperature;
}
