#include <Arduino.h>


#define SEN Serial1
#define RXD2 18
#define TXD2 19

void sensor_auto_update();
void sensor_init();
void read_sensor();
void clear_uart_buffer();
void check_sensor_type();