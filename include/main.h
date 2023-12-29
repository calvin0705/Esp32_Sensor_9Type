// #include <WebServer.h>
#include <PubSubClient.h>
#include <WiFiClient.h>

char* flashRead(int i);
void callback(char* topic, byte* message, unsigned int length);
void connect_mqttServer();
void client_publish(const char *topic, const char *payload);
void client_conn();
void connect_mqttServer();

// bool flag_html_write;