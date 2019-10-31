#include <time.h>
#include <SPI.h>
#include "RF24.h"

bool presence = false;

float randomize() {
  return rand() / (float) RAND_MAX;
}

void updatePresence() {
  presence = randomize() > 0.80;
}


// -----------------------------------------------------------------------------
RF24 radio(7, 8);

#define PIPE_TX 0xF0F0F0F0D2LL
#define PIPE_RX 0xF0F0F0F0E1LL
char PROTOCOL_ID = 'H';
char radioNumber = 'b';
char destiny = 's';

String encapsulate(String message, char destiny) {
  String response;
  response += PROTOCOL_ID;  // Protocolo
  response += destiny;      // Destino
  response += radioNumber;  // Origem
  response += message;      // Mensagem
  return response;
}

void setup() {
  srand ( millis() );
  Serial.begin(115200);
  radio.begin();

  radio.setPALevel(RF24_PA_HIGH);
  radio.setAutoAck(false);
  radio.setChannel(37);

  radio.openWritingPipe(PIPE_TX);
  radio.openReadingPipe(1, PIPE_RX);
  
  radio.startListening();
}

void waitCarrier() {
  do {
    radio.startListening();
    delay(128);
    radio.stopListening();
  } while(radio.testCarrier());
}

void send(String package) {
  radio.stopListening();
  delay(50);
  radio.startWrite(package.c_str(), package.length(), false);
  delay(50);
  radio.startListening();
}

void loop() {
  String message = String(presence);
  char response[32] = {0};

  radio.startListening();
  delay(50);

  while (radio.available()) {
    radio.read(&response, sizeof(response));
  }
  delay(50);
  //Serial.println("Loop");

  if(response[0] == PROTOCOL_ID) {
    Serial.println("LOG: Mensagem da rede: " + String(response));
    if (response[1] == radioNumber) {
      if (response[2] == '*') {   // Se for do AP, enviar
        if (message.length() != 0) {
          updatePresence();
          String package = encapsulate(message, destiny);
          waitCarrier();
          send(package);

          Serial.println("LOG: Enviado: " + String(package).substring(3));
        }
      } else {  // Caso contrário, imprima
        Serial.print("LOG: Recebido: ");
        Serial.println(String(response).substring(3));
      }
    }
  }
}
