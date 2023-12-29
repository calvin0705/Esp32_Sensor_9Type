#include <iostream>
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

char data6[20];

char* trasferChar = {};
char trasferArry[20] = "";
String str_data_point;
float float_data_point;
String str_topic_sn;

String SPIFFS_read(String file_name_num, char* data_point);

void SPIFFS_begin(){
  if (SPIFFS.begin(true)){
    Serial.println("SPIFFS file system create successful");
  }
}

void SPIFFS_write(String file_name_num, char* data_point){
  File file1_w = SPIFFS.open(file_name_num, FILE_WRITE);
  delay(10);

  Serial.println("******************************************************************111111111111");
  Serial.println("******************************************************************111111111111");
  Serial.println("******************************************************************111111111111");

  Serial.printf("file_name_num ===========>>> %s \n ", file_name_num);
  Serial.printf("data_point111 ===========>>> %s \n ", data_point);

  file1_w.write((uint8_t *)data_point, strlen(data_point));
  delay(10);

  file1_w.close();
  delay(10);
}

String SPIFFS_read(String file_name_num, char* data_point){
  File file1_r = SPIFFS.open(file_name_num, FILE_READ);
  int n = 0;
  while (file1_r.available())
  {
    data_point[n] = file1_r.read();
    n = n + 1;
  }
  file1_r.close();
  delay(10);
  strcpy(trasferArry, data_point);

  Serial.println("******************************************************************222222222222");
  Serial.println("******************************************************************222222222222");
  Serial.println("******************************************************************222222222222");
  Serial.printf("data_point ===========>>> %s \n ", data_point);
  Serial.printf("trasferArry ===========>>> %s \n ", trasferArry);

  str_data_point = String(trasferArry);

  return str_data_point;
}

float SPIFFS_read_float(String file_name_num, char* data_point){
  File file1_r = SPIFFS.open(file_name_num, FILE_READ);
  int n = 0;
  while (file1_r.available())
  {
    data_point[n] = file1_r.read();
    n = n + 1;
  }
  file1_r.close();
  strcpy(trasferArry, data_point);

  float_data_point = atof(trasferArry);

  return float_data_point;
}

void SPIFFS_totol_size(){
  Serial.printf("SPIFFS totalBytes: %d (Bytes)\n", SPIFFS.totalBytes());
  Serial.printf("SPIFFS usedBytes: %d (Bytes)\n", SPIFFS.usedBytes());
}