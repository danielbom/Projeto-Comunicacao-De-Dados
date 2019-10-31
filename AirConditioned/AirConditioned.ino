#include <time.h>
#include <SPI.h>
#include "RF24.h"

int cont = 0;

void updateTemperature(char boolSense) {
  if (boolSense == '1') {
    cont = 0;
    Serial.println("LOG: Ligado.");
  } else {
    cont++;
    if (cont == 5){
      Serial.println("LOG: Desligado.");
    }
  }
}



// -----------------------------------------------------------------------------
RF24 radio(7, 8);

#define PIPE_TX 0xF0F0F0F0D2LL
#define PIPE_RX 0xF0F0F0F0E1LL
char PROTOCOL_ID = 'H';
char radioNumber = 'e';
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
  srand ( time(NULL) );
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
  String message = String(cont);
  char response[32] = {0};

  radio.startListening();
  delay(50);

  while (radio.available()) {
    radio.read(&response, sizeof(response));
  }
  delay(50);
  //Serial.println("Loop");

  if(response[0] == PROTOCOL_ID) {
    Serial.println("LOG: Mensagem da rede " + String(response));
    if (response[1] == radioNumber || response[1] == 's') {
      if (response[2] == '*') {   // Se for do AP, enviar
        if (message.length() != 0) {
          String package = encapsulate(message, destiny);
          waitCarrier();
          send(package);

          Serial.println("LOG: Enviado "+ String(package).substring(3) );
        }
      } else {  // Caso contrário, imprima
        Serial.print("LOG: Recebido ");
        Serial.println(String(response).substring(3));
        updateTemperature(response[3]);
      }
    }
  }
}
