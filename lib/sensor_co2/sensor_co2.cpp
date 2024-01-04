#include <Arduino.h>
#include <PubSubClient.h>
#include <SPIFFS.h>
#include <main.h>
#include <WebServer.h>
#include <sensor_cmd.h>

// ==============================================
// Define Global Variable
// ==============================================


// ==============================================
// Define Main Loop Function
// ==============================================
String sensor_data_transfer()
{
  int a=0;
  int ppm=0;
  int ppm1=0;
  int ppm2=0;

  char str_a[100];
  char str_d[100];
  char str_mqtt[100];

  String incoming = "";
  String str_ppm;
  
  read_sensor();

  incoming  = SEN.readString();

  for(byte i=0; i<incoming.length(); i++)
  {
  Serial.print(incoming[i],HEX);
  Serial.write(' ');
  }
  Serial.println();
  
  ppm1 = incoming[2];
  ppm2 = incoming[3];
  ppm = ppm1*256+ppm2;
  str_ppm = String(ppm);

  delay(1000);

  Serial.print("Read Sensor(ppm) : ");
  Serial.println(ppm);

  if(ppm == 0)
  {
    return "NoSensor";
  }
  return str_ppm;
}

// ==============================================
// Define Setup Function
// ==============================================
void task_co2(String topic_sn) {
  String str_ppm;
  char str_a[100];

  connect_mqttServer();

  char ary_topic_1[20] = "";
  String str_topic_1 = "cvilux/CO2-";

  str_ppm = sensor_data_transfer();
  

  Serial.printf("topic_sn ==>> %s\n", topic_sn);

  str_topic_1 = str_topic_1 + topic_sn;
  str_topic_1.toCharArray(ary_topic_1, 20);
  
  sprintf(str_a, "%s",str_ppm);
  Serial.printf("str_a ==========>> %s\n", str_a);
  
  Serial.println("ary_topic_1 ========>> : " + String(ary_topic_1));

  if(str_ppm == "NoSensor")
  {
    Serial.println("No Sensor !!!");
  }
  else
  {
    client_publish(ary_topic_1, str_a);
  }
}