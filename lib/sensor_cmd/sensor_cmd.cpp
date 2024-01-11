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
  int i = 0;

  for(i=0;i<5;i++)
  {
    SEN.setTimeout(500);
    read_sensor();
    incoming  = SEN.readString();

    Serial.printf("clear_uart_buffer i ================>> %d \n", i);
  }
}

void check_sensor_type()
{
  int ppm=0;
  int ppm1=0;
  int ppm2=0;
  int sen_type=0;
  char str_a[100];
  String incoming = "";

  clear_uart_buffer();
  sensor_auto_update();
  clear_uart_buffer();
  sensor_auto_update();
  
  read_sensor();
  incoming  = SEN.readString();
  sen_type = incoming[1];
  if(sen_type == 23)
  {
    Serial.println("Sensor is CH2O ---------------------------!!!");
  }

  clear_uart_buffer();
  sensor_init();
  sensor_init();
  clear_uart_buffer();

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