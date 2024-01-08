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
float co_data_transfer()
{
  int a=0;
  int ppm=0;
  int ppm1=0;
  int ppm2=0;
  float f_ppm=0;
  int ser_type=0;

  char str_a[100];
  char str_d[100];
  char str_mqtt[100];

  String incoming = "";
  String str_ppm;
  
  read_sensor();
  incoming  = SEN.readString();
  ser_type = incoming[1];

  if(ser_type == 4)
  {
    read_sensor();
    incoming  = SEN.readString();
  }

  for(byte i=0; i<incoming.length(); i++)
  {
  Serial.print(incoming[i],HEX);
  Serial.write(' ');
  }
  Serial.println();
  
  ppm1 = incoming[2];
  ppm2 = incoming[3];
  f_ppm = (ppm1*256+ppm2)*0.1;
//   Serial.printf("f_ppm ============>> %f\n", f_ppm);
//   Serial.printf("f_ppm ============>> %f\n", f_ppm);
  str_ppm = String(ppm);

  delay(1000);

  Serial.print("Read Sensor(ppm) : ");
  Serial.println(ppm);

  if(f_ppm == 0)
  {
    return 0;
  }
  return f_ppm;
}

// ==============================================
// Define Setup Function
// ==============================================
void task_co(String topic_sn) {
  String str_ppm;
  char str_a[100];
  float f_ppm=0;

  connect_mqttServer();

  char ary_topic_1[20] = "";
  String str_topic_1 = "cvilux/CO-";

  f_ppm = co_data_transfer();
  Serial.printf("f_ppm ============>> %f\n", f_ppm);

  Serial.printf("topic_sn ==>> %s\n", topic_sn);

  str_topic_1 = str_topic_1 + topic_sn;
  str_topic_1.toCharArray(ary_topic_1, 20);
  
  sprintf(str_a, "%f",f_ppm);
  Serial.printf("str_a 123==========>> %s\n", str_a);
  
  Serial.println("ary_topic_1 123========>> : " + String(ary_topic_1));

//   if(str_ppm == 0)
//   {
//     Serial.println("No Sensor !!!");
//   }
//   else
//   {
//     client_publish(ary_topic_1, str_a);
//   }

  client_publish(ary_topic_1, str_a);
}