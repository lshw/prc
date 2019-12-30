#define _24V_OUT 3
#define PC_RESET A4
#define PC_POWER A5
#define LED 13
#define NET_RESET 4
#define W5500_CS 10
void setup() {
//Serial.begin(57600);
pinMode(_24V_OUT,OUTPUT);
digitalWrite(_24V_OUT,HIGH);
pinMode(PC_RESET,OUTPUT);
digitalWrite(PC_RESET,HIGH);
pinMode(PC_POWER,OUTPUT);
digitalWrite(PC_POWER,LOW);
pinMode(LED,OUTPUT);
digitalWrite(LED,LOW);
pinMode(NET_RESET,OUTPUT);
digitalWrite(NET_RESET,HIGH);

}

void loop() {
digitalWrite(PC_POWER,HIGH);
digitalWrite(PC_RESET,LOW);
digitalWrite(LED,HIGH);
delay(500);
digitalWrite(LED,LOW);
delay(500);
digitalWrite(PC_POWER,LOW);
digitalWrite(PC_RESET,HIGH);
digitalWrite(LED,HIGH);
delay(500);
digitalWrite(LED,LOW);
delay(500);
digitalWrite(_24V_OUT,!digitalRead(_24V_OUT));
}
