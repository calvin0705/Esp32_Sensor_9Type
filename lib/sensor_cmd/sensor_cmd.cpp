#include <Arduino.h>
#include <sensor_cmd.h>

// ==============================================
// Define Sensor Initial
// ==============================================

// per 1s update sensor data
// void sensor_auto_update()
// {
//     SEN.write(0xFF);
//     SEN.write(0x01);
//     SEN.write(0x78);
//     SEN.write(0x40);
//     SEN.write(0x00);
//     SEN.write(0x00);
//     SEN.write(0x00);
//     SEN.write(0x00);
//     SEN.write(0x47);
// }

// initial to read and return data
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