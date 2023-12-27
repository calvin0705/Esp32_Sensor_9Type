#include <Arduino.h>
#include <iostream>
#include <PubSubClient.h>
#include <SPIFFS.h>
#include <main.cpp>

#define SEN Serial1
#define RXD2 18
#define TXD2 19

#define AP_SSID_2 "廣州炒麵_2"

// ==============================================
// Define Global Variable
// ==============================================
const int REG = 0;

int a=0;
int ppm=0;
int ppm1=0;
int ppm2=0;

// ==============================================
// Define Sensor Initial
// ==============================================
void sensor_init()
{
  // 切換問答模式
  SEN.write(0xFF);
  SEN.write(0x01);
  SEN.write(0x78);
  SEN.write(0x41);
  SEN.write(0x00);
  SEN.write(0x00);
  SEN.write(0x00);
  SEN.write(0x00);
  SEN.write(0x46);
}

void read_sensor()
{
  SEN.write(0xFF);
  SEN.write(0x01);
  SEN.write(0x86);
  SEN.write(0x00);
  SEN.write(0x00);
  SEN.write(0x00);
  SEN.write(0x00);
  SEN.write(0x00);
  SEN.write(0x79);
}

// ==============================================
// Define Main Loop Function
// ==============================================

String sensor_data_transfer()
{
  char str_a[100];
  char str_d[100];
  char str_mqtt[100];

  String incoming = "";
  String str_ppm;
  
  read_sensor();

  incoming  = SEN.readString();
  
  ppm1 = incoming[2];
  ppm2 = incoming[3];
  ppm = ppm1*256+ppm2;
  str_ppm = String(ppm);

  delay(1000);

  Serial.print("Read Sensor(ppm) : ");
  Serial.println(ppm);

  return str_ppm;
}

// ==============================================
// Define Setup Function
// ==============================================
const int len = 64;    // flashWrite, flashRead -> i = 0 to 63
const uint32_t addressStart = 0x3FA000; 

char* flashRead2(int i) {      // i = 0 to 63
  uint32_t flashAddress = addressStart + i*len;
  static char buff_read[len];
  if (ESP.flashRead(flashAddress,(uint32_t*)buff_read, sizeof(buff_read)-1)) {
    return buff_read;
  } else  
    return 0;  
}


void callback2(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

#include <WiFiManager.h>
#include <WebServer.h>

WiFiClient espClient2;
PubSubClient client2(espClient2);

char mqtt_server2[40];

// void connect_mqttServer2() {

//   Serial.println("mqtt_server : " + String("192.168.107.203"));
//   client2.setServer("192.168.107.203",1883);
//   client2.setCallback(callback2);

//   Serial.print("Attempting MQTT connection...");

//   if (client2.connect("ESP32_client1")) { // !!! Change the name of client here if multiple ESP32 are connected
//     Serial.println("connected");
//     client2.subscribe("rpi/broadcast");
//   } 
//   else {
//     Serial.print("failed, rc=");
//     Serial.print(client2.state());
//     Serial.println(" trying again in 2 seconds");
//     delay(5000);

//     ESP.restart();
//   }
// }

WiFiManager wm2;

void connect_mqttServer2() {
  strcpy(mqtt_server2, flashRead2(0));

  Serial.println("mqtt_server : " + String(mqtt_server2));
  client2.setServer(mqtt_server2,1883);
  client2.setCallback(callback2);

  Serial.print("Attempting MQTT connection...");

  if (client2.connect("ESP32_client1")) { // !!! Change the name of client here if multiple ESP32 are connected
    Serial.println("connected");
    client2.subscribe("rpi/broadcast");
  } 
  else {
    Serial.print("failed, rc=");
    Serial.print(client2.state());
    Serial.println(" trying again in 2 seconds");
    delay(5000);

    ESP.restart();

    // if (!wm2.startConfigPortal(AP_SSID_2,"00000000")) {
    //     Serial.println("sensor_co2 ~~~~~~~ failed to connect or hit timeout");
    //     delay(500);
    //     // ESP.restart();
    // } else {
    //   Serial.println("sensor_co2 ~~~~~~~ connected...yeey :)");
    // }
  }
}

void task_co2(String topic_sn) {
  String str_ppm;
  char str_a[100];

  if (!client2.connected()) 
  {
    connect_mqttServer2();
  }
  client2.loop();

  // strcpy("192.168.107.203", flashRead(0));

  char ary_topic_1[20] = "";
  String str_topic_1 = "cvilux/CO2-";

  str_ppm = sensor_data_transfer();
  Serial.printf("topic_sn ==>> %s\n", topic_sn);

  str_topic_1 = str_topic_1 + topic_sn;
  str_topic_1.toCharArray(ary_topic_1, 20);

  sprintf(str_a, "%s",str_ppm);
  
  Serial.println("ary_topic_1 ========>> : " + String(ary_topic_1));
  client2.publish(ary_topic_1, str_a); //topic name and send value.
}