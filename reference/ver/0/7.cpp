using namespace std;

#include <iostream>
#include "calculator.h"

#include <TaskScheduler.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <Adafruit_AHTX0.h>
#include "FS.h"
#include "SPIFFS.h"

Scheduler runner;

// ==================================================
// Flash Read / Write
// ==================================================
String String_mqtt;
bool First_set_mqtt = true;

const int len = 64;    // flashWrite, flashRead -> i = 0 to 63
const uint32_t addressStart = 0x3FA000; 
const uint32_t addressEnd   = 0x3FAFFF;

void flashWrite(char data[len], int i) {      // i = 0 to 63
  uint32_t flashAddress = addressStart + i*len;
  char buff_write[len];
  strcpy(buff_write, data);
  if (ESP.flashWrite(flashAddress,(uint32_t*)buff_write, sizeof(buff_write)-1))
    Serial.printf("address: %p write \"%s\" [ok]\n", flashAddress, buff_write);
  else 
    Serial.printf("address: %p write \"%s\" [error]\n", flashAddress, buff_write);
}

char* flashRead(int i) {      // i = 0 to 63
  uint32_t flashAddress = addressStart + i*len;
  static char buff_read[len];
  if (ESP.flashRead(flashAddress,(uint32_t*)buff_read, sizeof(buff_read)-1)) {
    return buff_read;
  } else  
    return 0;  
}

void flashErase() {
  if (ESP.flashEraseSector(addressStart / 4096))
    Serial.println("\nErase [ok]");
  else
    Serial.println("\nErase [error]");
}

// ==================================================
// SPIFFS Define
// ==================================================
char data1[20] = "";
char data2[20] = "";
float sensor_correction_Float = 0.0;
float sensor_correction_Float2 = 0.0;
String sensor_correction_String;
String sensor_correction_String2;
bool flag_html_write = false;

void SPIFFS_begin(){
  if (SPIFFS.begin(true)){
    Serial.println("SPIFFS file system create successful");
  }
}

// data1 "/test1.txt"
void SPIFFS_file1_write(){
  File file1_w = SPIFFS.open("/test1.txt", FILE_WRITE);
  file1_w.write((uint8_t *)data1, strlen(data1));
  file1_w.close();

  Serial.print("SPIFFS_file1_write ***************************************** !!!!! \n");
}

void SPIFFS_file1_read(){
  File file1_r = SPIFFS.open("/test1.txt", FILE_READ);
  // Serial.print("test1.txt file message : ");

  int n = 0;
  while (file1_r.available())
  {
    // Serial.print((char)file1_r.read()); // debug show only
    data1[n] = file1_r.read();
    n = n + 1;
  }
  sensor_correction_Float = atof(data1);
  Serial.printf("sensor_correction_Float111 ==>> %f \n", sensor_correction_Float);
  file1_r.close();
}

// data2 "/test2.txt"
void SPIFFS_file2_write(){
  File file1_w = SPIFFS.open("/test2.txt", FILE_WRITE);
  file1_w.write((uint8_t *)data2, strlen(data2));
  file1_w.close();
}

void SPIFFS_file2_read(){
  File file1_r = SPIFFS.open("/test2.txt", FILE_READ);
  // Serial.print("test1.txt file message : ");

  int n = 0;
  while (file1_r.available())
  {
    // Serial.print((char)file1_r.read()); // debug show only
    data2[n] = file1_r.read();
    n = n + 1;
  }
  
  sensor_correction_Float2 = atof(data2);
  Serial.printf("sensor_correction_Float222 ==>> %f \n", sensor_correction_Float2);
  file1_r.close();
}

void SPIFFS_totol_size(){
  Serial.printf("SPIFFS totalBytes: %d (Bytes)\n", SPIFFS.totalBytes());
  Serial.printf("SPIFFS usedBytes: %d (Bytes)\n", SPIFFS.usedBytes());
}

// ==================================================
// Wifi html keyin id/pwd
// ==================================================
#define AP_SSID_1 "瀚荃_ESG_Sensor_1"
#define AP_SSID_2 "瀚荃_ESG_Sensor_2"
#define AP_PWD  "00000000"
#define TRIGGER_PIN 0

bool wm_nonblocking = false;

WiFiManager wm;
WiFiManagerParameter custom_field_1;
WiFiManagerParameter custom_field_2;
WiFiManagerParameter custom_field_3;

WebServer server(80);
char mqtt_server[40];
char sensor_correction[10];
char sensor_correction2[10];

unsigned int  timeout   = 120;
unsigned int  startTime = millis();
bool portalRunning      = false;
bool startAP = true;

void checkButton(){
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    delay(50);
    if( digitalRead(TRIGGER_PIN) == LOW ){
      Serial.println("Button Pressed");
      delay(1000);
      if( digitalRead(TRIGGER_PIN) == LOW ){
        Serial.println("Button Held");
        Serial.println("Erasing Config, restarting");
        wm.resetSettings();
        ESP.restart();
      }

      Serial.println("Starting config portal");
      wm.setConfigPortalTimeout(120);
      
      if (!wm.startConfigPortal("淡水河_2","00000000")) {
        Serial.println("failed to connect or hit timeout");
        delay(3000);
        // ESP.restart();
      } else {
        Serial.println("connected...yeey :)");
      }
    }
  }
  delay(1000);
}

String getParam(String name){
  String value;
  if(wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void saveParamCallback(){
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM customfieldid_1 = " + getParam("customfieldid_1"));
  Serial.println("PARAM customfieldid_2 = " + getParam("customfieldid_2"));
  Serial.println("PARAM customfieldid_3 = " + getParam("customfieldid_3"));

  String_mqtt = getParam("customfieldid_1");
  sensor_correction_String = getParam("customfieldid_3");
  flag_html_write = true;
}

void Wifi_Setup() {
  wm.setTitle("瀚荃集團");
  WiFi.mode(WIFI_STA);
  // Serial.setDebugOutput(true);
  // pinMode(TRIGGER_PIN, INPUT);

  if(wm_nonblocking) wm.setConfigPortalBlocking(false);
  int customFieldLength = 40;

  new (&custom_field_1) WiFiManagerParameter("customfieldid_1", "mqtt_server_ip", "", customFieldLength,"placeholder=\"\"");
  wm.addParameter(&custom_field_1);
  wm.setSaveParamsCallback(saveParamCallback);

  new (&custom_field_2) WiFiManagerParameter("customfieldid_2", "mqtt_topic", "", customFieldLength,"placeholder=\"\"");
  wm.addParameter(&custom_field_2);
  wm.setSaveParamsCallback(saveParamCallback);

  new (&custom_field_3) WiFiManagerParameter("customfieldid_3", "sensor_correction", "", customFieldLength,"placeholder=\"\"");
  wm.addParameter(&custom_field_3);
  wm.setSaveParamsCallback(saveParamCallback);

  std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
  wm.setMenu(menu);
  wm.setClass("invert");

  wm.setConfigPortalTimeout(120);
  wm.setConnectTimeout(10);

  bool res;
  res = wm.autoConnect("淡水河_1","00000000");

  if(!res) {
    Serial.println("Failed to connect or hit timeout");
    // ESP.restart();
  } 
  else {
    Serial.println("connected...yeey :)");
  }
}

// ==================================================
// MQTT Setup
// ==================================================
// const char* mqtt_server_1 = "192.168.18.203";   ### tfa00
// const char* mqtt_server_1 = "192.168.131.203";  ### smart phone

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void connect_mqttServer() {

  // wm.autoConnect(AP_SSID, AP_PWD);

  if (!client.connected()) {
    // strcpy(mqtt_server, custom_mqtt_server.getValue());
    // String_mqtt = getParam("customfieldid_1");
    String_mqtt.toCharArray(mqtt_server, 40);

    if((String_mqtt.indexOf('.') != -1) && (First_set_mqtt == true)){
      Serial.println("flashErase / flashWrite");
      flashErase();
      flashWrite(mqtt_server, 0);
      delay(10);
      First_set_mqtt = false;
    }

    strcpy(mqtt_server, flashRead(0));
    Serial.println("mqtt_server : " + String(mqtt_server));
    client.setServer(mqtt_server,1883);
    client.setCallback(callback);

    //now attemt to connect to MQTT server
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32_client1")) { // !!! Change the name of client here if multiple ESP32 are connected
      Serial.println("connected");
      client.subscribe("rpi/broadcast");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 2 seconds");
      delay(1000);
    }
  }
}

// ==================================================
// AHT20_Setup
// ==================================================
Adafruit_AHTX0 aht;

void AHT20_Setup() {
  // AHT20
  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("");
  Serial.println("AHT10 or AHT20 found");
  Serial.println("");
}

void task_temp() {
  int sensor_correction_int = 0;

  if(flag_html_write == true){
    // sensor_correction_String = getParam("customfieldid_3");
    sensor_correction_String.toCharArray(data1, 20);
    sensor_correction_Float = sensor_correction_String.toFloat();

    Serial.printf("sensor_correction ............................ ==>> %s \n", sensor_correction);
    Serial.printf("sensor_correction_String ..................... ==>> %s \n", sensor_correction_String);
    Serial.println("wm.getLastConxResult() : " + wm.getLastConxResult());
    Serial.println("wm.getConfigPortalActive() : " + wm.getConfigPortalActive());
    Serial.println("wm.getWiFiIsSaved() : " + wm.getWiFiIsSaved());

    if(sensor_correction_String != ""){
      SPIFFS_file1_write();
      delay(10);
      flag_html_write = false;
    }
  }

  SPIFFS_file1_read();
  SPIFFS_file2_read();
  delay(10);

  // ==================================================
  if (!client.connected()) 
  {
    connect_mqttServer();
  }
  client.loop();

  // temp sensor read
  char str_a[100];
  char str_b[100];

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  float temp1 = 0.0;
  temp1 = temp.temperature;
  temp1 = temp1 + sensor_correction_Float;

  float humi1 = 0.0;
  humi1 = humidity.relative_humidity;
  humi1 = humi1 + sensor_correction_Float2;

  sprintf(str_a, "%f",temp1);
  sprintf(str_b, "%f",humi1);

  // Serial.println("task_temp *************************************************");
  Serial.println(str_a);
  Serial.println(str_b);

  client.publish("cvilux/temp3", str_a); //topic name and send value.
  delay(1); 
  client.publish("cvilux/humi3", str_b); //topic name and send value.
}

// ==================================================
// ISR Interrupt
// ==================================================
const uint8_t InterruptPin1 = 32;
bool Request1;

const uint8_t InterruptPin2 = 33;
bool Request2;

const uint8_t InterruptPin3 = 34;
bool Request3;

const uint8_t InterruptPin4 = 35;
bool Request4;

void IRAM_ATTR isr1() {
 Request1 = true;
}

void IRAM_ATTR isr2() {
 Request2 = true;
}

void IRAM_ATTR isr3() {
 Request3 = true;
}

void IRAM_ATTR isr4() {
 Request4 = true;
}

void setup_isr() {
  pinMode(InterruptPin1, INPUT);
  attachInterrupt(InterruptPin1, isr1, RISING);

  pinMode(InterruptPin2, INPUT);
  attachInterrupt(InterruptPin2, isr2, RISING);

  pinMode(InterruptPin3, INPUT);
  attachInterrupt(InterruptPin3, isr3, RISING);

  pinMode(InterruptPin4, INPUT);
  attachInterrupt(InterruptPin4, isr4, RISING);
}

void task_isr() {
  if (Request1){
      Serial.println("Interrupt Request Received! 1111111111111");
      Request1 = false;
      SPIFFS_file1_read();
      sensor_correction_Float = sensor_correction_Float + 1;
      sensor_correction_String = String(sensor_correction_Float);
      sensor_correction_String.toCharArray(data1, 100);
      
      SPIFFS_file1_write();
  }

  if (Request2){
      Serial.println("Interrupt Request Received! 222222222222222");
      Request2 = false;
      SPIFFS_file1_read();
      sensor_correction_Float = sensor_correction_Float - 1;
      sensor_correction_String = String(sensor_correction_Float);
      sensor_correction_String.toCharArray(data1, 100);
      
      SPIFFS_file1_write();
  }

  if (Request3){
      Serial.println("Interrupt Request Received! 3333333333333");
      Request3 = false;
      SPIFFS_file2_read();
      sensor_correction_Float2 = sensor_correction_Float2 + 1;
      sensor_correction_String2 = String(sensor_correction_Float2);
      sensor_correction_String2.toCharArray(data2, 100);
      
      SPIFFS_file2_write();
  }

  if (Request4){
      Serial.println("Interrupt Request Received! 44444444444444");
      Request4 = false;
      SPIFFS_file2_read();
      sensor_correction_Float2 = sensor_correction_Float2 - 1;
      sensor_correction_String2 = String(sensor_correction_Float2);
      sensor_correction_String2.toCharArray(data2, 100);
      
      SPIFFS_file2_write();
  }
}

// ==================================================
// Task Function Define
// ==================================================
// Scheduler runner;
void t1Callback() { 
  task_isr();
  // Serial.println("t1 ======================");
}

void t2Callback() {
  task_temp();
  Serial.println("t2 ======================");
}

void t3Callback() {
  int result = 0;

  result = myFunction(5, 8);

  delay(2000);
  Serial.printf("myFunction ==>> %d \n", result);
  Serial.println("t3 ======================");
}

Task t1(300, TASK_FOREVER, &t1Callback);
Task t2(1500, TASK_FOREVER, &t2Callback);
Task t3(1000, TASK_FOREVER, &t3Callback);

void task_setup() {
  runner.init();
  runner.addTask(t1);
  runner.addTask(t2);
  runner.addTask(t3);
  t1.enable();
  t2.enable();
  t3.enable();
}
// Task Function END of Line
// ==================================================

// ==================================================
// System Setup
// ==================================================
void setup () {
  Serial.begin(115200);
  task_setup(); // Run Task
  Wifi_Setup(); // Wifi html keyin id/pwd
  AHT20_Setup();
  setup_isr();

  SPIFFS_begin();
}

// ==================================================
// System Main Loop
// ==================================================
void loop () {
  runner.execute();
  checkButton();
  delay(100);
}