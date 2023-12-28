#include <iostream>
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

char* trasferChar = {};
char trasferArry[20] = "";
String str_data_point;

void SPIFFS_write(String file_name_num, char* data_point){
  File file1_w = SPIFFS.open(file_name_num, FILE_WRITE);
  delay(10);
//   file1_w.write((uint8_t *)data3, strlen(data3));
  file1_w.write((uint8_t *)data_point, strlen(data_point));
  delay(1000);
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
  strcpy(trasferArry, data_point);

  str_data_point = String(trasferArry);

  return str_data_point;
}