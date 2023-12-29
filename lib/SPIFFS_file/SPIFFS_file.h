#include <iostream>
#include <Arduino.h>

char data1[20];
char data2[20];
char data3[20];
char data4[20];
char data5[20];

void SPIFFS_begin();
void SPIFFS_write(String file_name_num, char* data_point);
String SPIFFS_read_string(String file_name_num);
float SPIFFS_read_float(String file_name_num);
void SPIFFS_totol_size();