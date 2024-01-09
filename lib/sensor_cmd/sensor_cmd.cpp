#include <Arduino.h>
#include <sensor_cmd.h>

// ==============================================
// Define Sensor Initial
// ==============================================

// per 1s update sensor data
void sensor_auto_update()
{
    SEN.write(0xFF);
    SEN.write(0x01);
    SEN.write(0x78);
    SEN.write(0x40);
    SEN.write(0x00);
    SEN.write(0x00);
    SEN.write(0x00);
    SEN.write(0x00);
    SEN.write(0x47);
}

// initial to read and return data Q/A mode
void sensor_init()
{
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

void clear_uart_buffer()
{
  String incoming = "";
  
  read_sensor();
  incoming  = SEN.readString();
  delay(100);

  read_sensor();
  incoming  = SEN.readString();
  delay(100);
}

void check_sensor_type()
{
  sensor_auto_update();
  sensor_auto_update();
  sensor_auto_update();

  int ppm=0;
  int ppm1=0;
  int ppm2=0;
  int sen_type=0;

  char str_a[100];

  String incoming = "";
  
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

  Serial.printf("check_sensor_type ppm ================================>> %d \n", ppm);
}