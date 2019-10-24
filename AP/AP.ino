#include <SPI.h>
#include "RF24.h"

char radioNumber = '*';
char clients[3] = {'b','0','e'};

int timeout = 500;
int delayTime = 500;

RF24 radio(7,8);

#define PIPE_RX 0xF0F0F0F0D2LL
#define PIPE_TX 0xF0F0F0F0E1LL
char PROTOCOL_ID = 'H';

char response[32];

// ----------------------- //
//          Utils          //
// ----------------------- //
String encapsulate(String message, char destiny) {
  String response;
  response += PROTOCOL_ID;
  response += destiny;
  response += radioNumber;
  response += message;
  return response;
}

void testCarrier() {
  do {
    radio.startListening();
    delay(128);
    radio.stopListening();
  } while(radio.testCarrier());
}

// ----------------------- //
//   Protocol controller   //
// ----------------------- //
void send(String package) {
  radio.stopListening();
  delay(50);
  radio.startWrite(package.c_str(), package.length(), false);
  delay(50);
  radio.startListening();
}

void route() {
  String package;
  Serial.println("Recebendo para rotear");
  while (radio.available()) {
    radio.read(&response, sizeof(response));
  }

  if (response[0] != '\0') {
    Serial.print("Mensagem: ");
    Serial.println(response);
    package = response;
    if (response[0] == PROTOCOL_ID) {
      send(package);
      Serial.print("Roteei: ");
      Serial.println(package);
    } else {
      Serial.println("Pacote de outra rede");
    }
    Serial.println();
    response[0] = '\0';  
  }
}

bool received() {
  radio.startListening();
  unsigned long startListenTime = millis();
  while (!radio.available()) {
    if ((millis() - startListenTime) > timeout) {
      Serial.println("Timeout\n");
      return false;
    }
  }
  return true;
}

void tokenRing() {
  for(int i = 0; i < 3; i++) {
    String package = encapsulate("", clients[i]);
    Serial.print("Token: ");
    Serial.println(package);
    for(int tries = 0; tries < 3; tries++) {
      testCarrier();
      send(package);
      if (received()) {
        route();
        break;
      }
    }
  }
  Serial.println();
}

// ----------------------- //
//          Start          //
// ----------------------- //
void setup() {
  Serial.begin(115200);
  radio.begin();
  
  radio.setPALevel(RF24_PA_HIGH);
  radio.setAutoAck(false);

  // radio.setChannel(37);
  
  radio.openWritingPipe(PIPE_TX);
  radio.openReadingPipe(1, PIPE_RX);
}

void loop() {
  tokenRing();
  delay(delayTime);
}
