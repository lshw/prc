#define debug  false
#define _24V_OUT 3
#define PC_RESET A4
#define PC_POWER A5
#define LED 13
#define NET_RESET 4

#include <EEPROM.h>
#include <avr/wdt.h>
#include <SPI.h>
#include <Ethernet2.h>

//eeprom 内存分配
enum
{
  MAC0, //0
  MAC1, //1
  MAC2,   //2,序列号存放地址
  MAC3, //3,版本
  MAC4, //4
  MAC5, //5
  IP0,
  IP1,
  IP2,
  IP3,
  NETMARK,//len
  GW0,
  GW1,
  GW2,
  GW3,
  SPEED,
  COMSET_H,
  COMSET_L
};

//设置
struct setting {
  int mac[6];
  char ip[4];
  char getway[4];
  char  subnet[4];
} setting;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01
};

IPAddress ip(192, 168, 12, 177);
IPAddress gateway(192, 168, 12, 1);
IPAddress subnet(255, 255, 255, 0);

EthernetServer server(23);

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
  Serial.print(F("#Server ip"));
  Serial.println(Ethernet.localIP());
  setup_watchdog(WDTO_30MS);
  if (debug)
    displaybz();
  OSCCAL = 147; //校准rc震荡器
}
boolean alreadyConnected = false;
EthernetClient client;
uint32_t blank_time = 0;
void loop() {
  char ch;
  if (alreadyConnected) {
    if (!client.connected()) {
      client.stop();
      alreadyConnected = false;
    }
  }
  if (!alreadyConnected) {
    client = server.available();
    if (!client) return;
    alreadyConnected = true;
    blank_time = millis() + 100; //10毫秒内收到的乱码，忽略
    if (debug)
      Serial.println(F("\r\n#new client in"));
  }

  while (client.available() > 0) { //tcp有数据进来
    ch = client.read();
    if (blank_time > millis()) continue; //开始10ms收到的内容会被忽略
    if (debug) {
      Serial.write(' ');
      Serial.write('[');
      if (ch >= 0 && ch < 0x10) Serial.write('0');
      Serial.print((uint8_t)ch, HEX);
      Serial.write(']');
    }
    Serial.write(ch);
  }
  while (Serial.available()) {
    ch = Serial.read();
    client.write(ch);
  }
}

//看门狗中断做定时任务 30ms 1次
uint16_t volatile sec = 0, ms = 0;
uint16_t volatile dogcount = 0; //超时重启，主程序循环清零，不清零的话100秒重启系统
int16_t volatile pc_reset_on = 0; //按下pc_reset键的ms时长
int16_t volatile pc_power_on = 0; //按下pc_power键的ms时长
ISR(WDT_vect) {
  dogcount++;  //30ms
  if (dogcount > 100000 / 30) asm volatile ("  jmp 0"); //100秒看门狗超时重启
  ms += 30;
  if (ms > 1000) {
    ms -= 1000;
    sec++;
    if (debug)
      if (sec % 10 == 0)
        Serial.println(sec);
  }
  //处理reset键，其它程序只要修改 pc_reset_on=300，就可以按下300ms
  if (pc_reset_on > 0) pc_reset_on -= 30; //30ms
  if (pc_reset_on > 0) { //reset开关按下
    if (digitalRead(PC_RESET) != HIGH)
      digitalWrite(PC_RESET, HIGH);
  } else { //reset开关松开
    if (digitalRead(PC_RESET) != LOW)
      digitalWrite(PC_RESET, LOW);
  }

  //处理reset键，其它程序只要修改 pc_power_on=300，就可以按下300ms
  if (pc_power_on > 0) pc_power_on -= 30; //30ms
  if (pc_power_on > 0) { //power开关按下
    if (digitalRead(PC_POWER) != HIGH)
      digitalWrite(PC_POWER, HIGH);
  } else { //power开关松开
    if (digitalRead(PC_POWER) != LOW)
      digitalWrite(PC_POWER, LOW);
  }
}

//设置看门狗定时中断时间ii=WDTO_15MS .... WDTO_8S
void setup_watchdog(int ii) {
  byte bb;
  if (ii > 9 ) ii = 9;
  bb = ii & 7;
  if (ii > 7) bb |= (1 << 5);
  bb |= (1 << WDCE);
  MCUSR &= ~(1 << WDRF);
  // start timed sequence
  WDTCSR |= (1 << WDCE) | (1 << WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
}

//校准rc振荡器
inline void displaybz() {
  uint8_t osc;
  delay(10);
  osc = OSCCAL;
  for (int8_t i = -20; i < 20; i++) {
    OSCCAL = osc + i;
    delay(100);
    Serial.print(F("OSC_OFFSET="));
    Serial.println(osc + i);
  }
  Serial.flush();
}
