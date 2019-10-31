#include <SPI.h>
#include "RF24.h"

int count = 0 ;

RF24 radio(7, 8);

#define PIPE_TX 0xF0F0F0F0D2LL
#define PIPE_RX 0xF0F0F0F0E1LL
char PROTOCOL_ID = 'H';
char radioNumber = '0';
char destiny = 'b';

String encapsulate(String message, char destiny) {
  String response;
  response += PROTOCOL_ID;  // Protocolo
  response += destiny;      // Destino
  response += radioNumber;  // Origem
  response += message;      // Mensagem
  return response;
}

void setup() {
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
  String message = String(count);
  char response[32] = {0};

  radio.startListening();
  delay(50);

  while (radio.available()) 
    radio.read(&response, sizeof(response));
  delay(50);

  if(response[0] == PROTOCOL_ID) {
    Serial.println("LOG: Mensagem da rede: " + String(response));
    if (response[1] == radioNumber || response[1] == 's') { // Mensagem enviada para mim
      switch (response[2]) {
        case 'b': // Mensagem do sensor de presença
          Serial.print("LOG: Recebido: ");
          Serial.print(String(response).substring(3));
          Serial.print(" ");
          Serial.println(count);
          count += response[3] == '0';
          if (count > 5) {
            Serial.println("LOG: Desligada.");
          } else {
            Serial.println("LOG: Ligada.");
          }
          break;
        case '*':
          String package = encapsulate(message, destiny);
          waitCarrier();
          send(package);
          Serial.println("LOG: Enviando: " + package.substring(3));
      }
    }
  }
}