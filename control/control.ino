#define _24V_OUT 3
#define PC_RESET A4
#define PC_POWER A5
#define LED 13
#define NET_RESET 4
#include <SPI.h>
#include <Ethernet2.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01
};
IPAddress ip(192, 168, 1, 177);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

EthernetServer server(23);

EthernetClient clients[4];


void setup() {
  pinMode(_24V_OUT, OUTPUT);
  digitalWrite(_24V_OUT, HIGH); //默认24V开启输出
  pinMode(PC_RESET, OUTPUT);
  digitalWrite(PC_RESET, LOW);
  pinMode(PC_POWER, OUTPUT);
  digitalWrite(PC_POWER, LOW);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  pinMode(NET_RESET, OUTPUT);
  digitalWrite(NET_RESET, HIGH);

  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();
  Serial.begin(115200);
  while (!Serial) {
    ;
  }

}

void loop() {
  EthernetClient client = server.available();

  // when the client sends the first byte, say hello:
  if (client) {

    boolean newClient = true;
    for (byte i = 0; i < 4; i++) {
      if (clients[i] == client) {
        newClient = false;
        break;
      }
    }

    if (newClient) {
      for (byte i = 0; i < 4; i++) {
        if (!clients[i] && clients[i] != client) {
          clients[i] = client;
          client.flush();
          Serial.println("We have a new client");
          client.print("Hello, client number: ");
          client.print(i);
          client.println();
          break;
        }
      }
    }

    if (client.available() > 0) {
      // read the bytes incoming from the client:
      char thisChar = client.read();
      // echo the bytes back to all other connected clients:
      for (byte i = 0; i < 4; i++) {
        if (clients[i] && (clients[i] != client)) {
          clients[i].write(thisChar);
        }
      }
      // echo the bytes to the server as well:
      Serial.write(thisChar);
    }
  }
  for (byte i = 0; i < 4; i++) {
    if (!(clients[i].connected())) {
      // client.stop() invalidates the internal socket-descriptor, so next use of == will allways return false;
      clients[i].stop();
    }
  }
}
