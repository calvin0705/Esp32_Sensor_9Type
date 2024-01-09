#include <Arduino.h>
#include <PubSubClient.h>
#include <SPIFFS.h>
#include <main.h>
#include <WebServer.h>
#include <sensor_cmd.h>

// ==============================================
// Define Global Variable
// ==============================================
bool is_sensor_date = true;

// ==============================================
// Define Main Loop Function
// ==============================================
float* pm25_data_transfer()
{
  int a=0;
  int ppm=0;
  int ppm1=0;
  int ppm2=0;
  static float f_ppm[5]={0};    // static is mustbe
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
  f_ppm[0] = (ppm1*256+ppm2);

  ppm1 = incoming[4];
  ppm2 = incoming[5];
  f_ppm[1] = (ppm1*256+ppm2);

  ppm1 = incoming[6];
  ppm2 = incoming[7];
  f_ppm[2] = (ppm1*256+ppm2);

  delay(1000);

  if(f_ppm[0] == 0)
  {
    is_sensor_date = false;
    return f_ppm;
  }
  return f_ppm;
}

// ==============================================
// Define Setup Function
// ==============================================
void task_pm25(String topic_sn) {
  String str_ppm;
  float* f_ppm={0};

  char str_a[50];
  char str_b[50];
  char str_c[50];

  connect_mqttServer();

  f_ppm = pm25_data_transfer();
  if(is_sensor_date == false)
  {
    Serial.println("No Sensor, Function will be return !!!");
    return;
  }  // If no sensor data, Return for do nothing

  Serial.printf("f_ppm ============>> %f\n", *(f_ppm));
  Serial.printf("f_ppm +1 ============>> %f\n", *(f_ppm+1));
  Serial.printf("f_ppm +2 ============>> %f\n", *(f_ppm+2));

  char ary_topic_1[20] = "";
  String str_topic_1 = "cvilux/PM1_0-";
  char ary_topic_2[20] = "";
  String str_topic_2 = "cvilux/PM2_5-";
  char ary_topic_3[20] = "";
  String str_topic_3 = "cvilux/PM10-";

  str_topic_1 = str_topic_1 + topic_sn;
  str_topic_1.toCharArray(ary_topic_1, 20);
  str_topic_2 = str_topic_2 + topic_sn;
  str_topic_2.toCharArray(ary_topic_2, 20);
  str_topic_3 = str_topic_3 + topic_sn;
  str_topic_3.toCharArray(ary_topic_3, 20);

  sprintf(str_a, "%f",*(f_ppm));
  sprintf(str_b, "%f",*(f_ppm+1));
  sprintf(str_c, "%f",*(f_ppm+2));

  if(*(f_ppm) <= 0)
  {
    Serial.println("No Sensor !!!");
  }
  else
  {
    client_publish(ary_topic_1, str_a);
    client_publish(ary_topic_2, str_b);
    client_publish(ary_topic_3, str_c);
  }
}