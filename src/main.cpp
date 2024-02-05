using namespace std;

#include <iostream>
#include <Arduino.h>

#include <TaskScheduler.h>
#include <WiFiManager.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <Adafruit_AHTX0.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WiFiClient.h>

#include <calculator.h>
#include <SPIFFS_file.h>

#include <sensor_cmd.h>
#include <sensor_co2.h>
#include <sensor_co.h>
#include <sensor_ch2o.h>
#include <sensor_pm25.h>

Scheduler runner;

char* flashRead(int i);
void callback(char* topic, byte* message, unsigned int length);
void connect_mqttServer();
void client_publish(const char *topic, const char *payload);
void client_conn();
void connect_mqttServer();

// ==================================================
// Flash Read / Write
// ==================================================
String String_mqtt;
bool First_set_mqtt = true;
bool is_count_30s = false;
int count_30 = 0;

const int len = 64;    // flashWrite, flashRead -> i = 0 to 63
const uint32_t addressStart = 0x3FA000; 
// const uint32_t addressStart = 0x3A0000; 

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
float sensor_correction_Float = 0.0;
float sensor_correction_Float2 = 0.0;
String sensor_correction_String;
String sensor_correction_String2;
String sensor_correction_String3;
bool flag_html_write = false;

char mqtt_topic_sn[20] = "";

// void SPIFFS_write(String file_name_num, char* data_point);
// SPIFFS_write("/test3.txt", data3);

// sensor_correction_Float2 = SPIFFS_read_float("/test2.txt", data2);
// String SPIFFS_read(String file_name_num, char* data_point);
// topic_sn = SPIFFS_read("/test3.txt", data3);

// ==================================================
// Sensor UART and hard define
// ==================================================
#define SEN Serial1
#define RXD2 18
#define TXD2 19

void serial_monitor()
{
  // Serial.begin(115200);
  SEN.begin(9600, SERIAL_8N1, RXD2, TXD2);
  SEN.setTimeout(1000);
}
// ==================================================
// Wifi html keyin id/pwd
// ==================================================
const char* ssid = "Aoe";
const char* password = "00000000";

int wifi_status;

void default_wifi() {

  wifi_status = WiFi.begin(ssid, password);
  Serial.printf("wifi_status1111 ===========>>> %d \n ", wifi_status);

  int c=0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(". \n");
    // Serial.println("A");
    c=c+1;
    if(c>10){
      // ESP.restart();
      // bypass to use html setup wifi.
      Serial.println("break");
      Serial.println("break");
      Serial.println("break");
      break;
    }
  }

  wifi_status = WiFi.status(); // "WL_CONNECTED = 3," "WL_DISCONNECTED = 6"
  Serial.printf("wifi_status2222 =========================>>> %d \n ", WiFi.status());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

#define AP_SSID_1 "CviCloud_ESG"
#define AP_SSID_2 "CviCloud_ESG"
#define AP_PWD  "00000000"
#define TRIGGER_PIN 0

bool wm_nonblocking = false;

WiFiManager wm;
WiFiManagerParameter custom_field_1;
WiFiManagerParameter custom_field_2;
WiFiManagerParameter custom_field_3;
WiFiManagerParameter custom_field_4;

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
      // delay(50);

      Serial.println("Starting config portal");
      wm.setConfigPortalTimeout(120);
      
      if (!wm.startConfigPortal(AP_SSID_2,"00000000")) {
        Serial.println("failed to connect or hit timeout");
        delay(500);
        // ESP.restart();
      } else {
        Serial.println("connected...yeey :)");
      }
    }
  }
  // delay(1000);
}

String getParam(String name){
  String value;
  if(wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

String topic_sn;
String topic_sn2;

void saveParamCallback(){
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM customfieldid_1 = " + getParam("customfieldid_1")); // mqtt server ip
  Serial.println("PARAM customfieldid_2 = " + getParam("customfieldid_2")); // mqtt topic
  // Serial.println("PARAM customfieldid_3 = " + getParam("customfieldid_3")); // sensor correction
  Serial.println("PARAM customfieldid_4 = " + getParam("customfieldid_4")); // mqtt topic

  String_mqtt              = getParam("customfieldid_1");
  topic_sn                 = getParam("customfieldid_2");
  // sensor_correction_String = getParam("customfieldid_3");
  topic_sn2                = getParam("customfieldid_4");

  // =====================
  // mqtt topic
  topic_sn.toCharArray(data3, 20);
  
  SPIFFS_write("/test3.txt", data3);
  topic_sn = SPIFFS_read_string("/test3.txt");
  Serial.printf("topic_sn ===========>>> %s \n ", topic_sn);

  // =====================
  // mqtt topic
  topic_sn2.toCharArray(data4, 20);
  
  SPIFFS_write("/test4.txt", data4);
  topic_sn2 = SPIFFS_read_string("/test4.txt");
  Serial.printf("topic_sn2 ===========>>> %s \n ", topic_sn2);

  // =====================
  // mqtt server ip
  String_mqtt.toCharArray(mqtt_server, 40);
  Serial.printf("mqtt_server ===>>>> %s \n", mqtt_server);
  if(String_mqtt.indexOf('.') != -1){
      Serial.println("flashErase / flashWrite");
      delay(10);
      flashErase();
      delay(10);
      flashWrite(mqtt_server, 0);
      delay(10);
      First_set_mqtt = false;
  }

  flag_html_write = true;
  Serial.println("saveParamCallback =========================  END ! ");
}

void Wifi_Setup() {
  wm.setTitle("WiFi Setup");
  WiFi.mode(WIFI_STA);

  if(wm_nonblocking) wm.setConfigPortalBlocking(false);
  int customFieldLength = 40;

  new (&custom_field_1) WiFiManagerParameter("customfieldid_1", "mqtt server ip", "", customFieldLength,"placeholder=\"\"");
  wm.addParameter(&custom_field_1);
  wm.setSaveParamsCallback(saveParamCallback);

  new (&custom_field_4) WiFiManagerParameter("customfieldid_2", "第幾顆(Number of Sensor)", "", customFieldLength,"placeholder=\"\"");
  wm.addParameter(&custom_field_2);
  wm.setSaveParamsCallback(saveParamCallback);

  // new (&custom_field_3) WiFiManagerParameter("customfieldid_3", "sensor_correction", "", customFieldLength,"placeholder=\"\"");
  // wm.addParameter(&custom_field_3);
  // wm.setSaveParamsCallback(saveParamCallback);

  new (&custom_field_2) WiFiManagerParameter("customfieldid_4", "序列號(Sensor Type)", "", customFieldLength,"placeholder=\"\"");
  wm.addParameter(&custom_field_4);
  wm.setSaveParamsCallback(saveParamCallback);

  // std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
  std::vector<const char *> menu = {"wifi","erase"};
  wm.setMenu(menu);

  wm.setConfigPortalTimeout(120);
  wm.setConnectTimeout(10);

  bool res;
  res = wm.autoConnect(AP_SSID_1,"00000000");

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
// const char* mqtt_server_1 = "192.168.29.203";

WiFiClient espClient;
PubSubClient client(espClient);

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
  // delay(1000);

  if (!client.connected()) {
    client_conn();
  }
}

// ==================================================
// AHT20_Setup
// ==================================================
Adafruit_AHTX0 aht;
bool is_AHT = false;

void AHT20_Setup() {
  // AHT20
  if (! aht.begin()) 
  {
    Serial.println("Could not find AHT? Check wiring");
  }
  else
  {
    Serial.println("");
    Serial.println("AHT10 or AHT20 found");
    Serial.println("");

    is_AHT = true;
  }
}

void read_sensor_sn() {
  topic_sn = SPIFFS_read_string("/test3.txt");
  topic_sn2 = SPIFFS_read_string("/test4.txt");
}

void client_publish(const char *topic, const char *payload){
  client.publish(topic, payload);
}

void client_conn(){
  strcpy(mqtt_server, flashRead(0));
    Serial.println("mqtt_server : " + String(mqtt_server));
    client.setServer(mqtt_server,1883);
    client.setCallback(callback);

  const char *client_mac; // uin client name for mqtt client device initial "client.connect(client_mac)"
  String str_client_mac;
  str_client_mac = ESP.getEfuseMac();

  client_mac= str_client_mac.c_str();

  Serial.printf("\nCHIP MAC: %012llx\n", ESP.getEfuseMac());
  // if (client.connect("ESP32_client1")) { // !!! 12345 Change the name of client here if multiple ESP32 are connected
  if (client.connect(client_mac)) {
    Serial.println("connected");
    client.subscribe("rpi/broadcast");
  } 
  else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" trying again in 2 seconds");
    delay(5000);
    First_set_mqtt = true;

    ESP.restart();
  }
}

void task_temp() {
  int sensor_correction_int = 0;

  if(flag_html_write == true){
    sensor_correction_String.toCharArray(data1, 20);
    sensor_correction_Float = sensor_correction_String.toFloat();

    if(sensor_correction_String != ""){
      SPIFFS_write("/test1.txt", data1);
      delay(10);
      flag_html_write = false;
    }
  }

  sensor_correction_Float  = SPIFFS_read_float("/test1.txt");
  sensor_correction_Float2 = SPIFFS_read_float("/test2.txt");
  delay(1);

  // ==================================================
  if (!client.connected()) 
  {
    connect_mqttServer();
  }
  client.loop();

  // temp sensor read
  char str_temp_a[100];
  char str_temp_b[100];

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  float temp1 = 0.0;
  temp1 = temp.temperature;
  temp1 = temp1 + sensor_correction_Float;

  float humi1 = 0.0;
  humi1 = humidity.relative_humidity;
  humi1 = humi1 + sensor_correction_Float2;

  sprintf(str_temp_a, "%f",temp1);
  sprintf(str_temp_b, "%f",humi1);

  Serial.println(str_temp_a);
  Serial.println(str_temp_b);

  // read_sensor_sn();
  Serial.printf("topic_sn ==>> %s\n", topic_sn);

  char ary_topic_1[20] = "";
  char ary_topic_2[20] = "";

  String str_topic_1 = "cvilux/";
  String str_topic_2 = "cvilux/";
  String str_topic_3 = "/temp/";
  String str_topic_4 = "/humi/";

  str_topic_1 = str_topic_1 + topic_sn2;
  str_topic_1 = str_topic_1 + str_topic_3;
  str_topic_1 = str_topic_1 + topic_sn;
  str_topic_1.toCharArray(ary_topic_1, 20);

  str_topic_2 = str_topic_2 + topic_sn2;
  str_topic_2 = str_topic_2 + str_topic_4;
  str_topic_2 = str_topic_2 + topic_sn;
  str_topic_2.toCharArray(ary_topic_2, 20);

  Serial.println("ary_topic_1 ========>> : " + String(ary_topic_1));
  Serial.println("ary_topic_2 ========>> : " + String(ary_topic_2));

  client.publish(ary_topic_1, str_temp_a); //topic name and send value.
  delay(1); 
  client.publish(ary_topic_2, str_temp_b); //topic name and send value.
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
    sensor_correction_Float = SPIFFS_read_float("/test1.txt");
    sensor_correction_Float = sensor_correction_Float + 1;
    sensor_correction_String = String(sensor_correction_Float);
    sensor_correction_String.toCharArray(data1, 100);

    SPIFFS_write("/test1.txt", data1);
  }

  if (Request2){
    Serial.println("Interrupt Request Received! 222222222222222");
    Request2 = false;
    sensor_correction_Float = SPIFFS_read_float("/test1.txt");
    sensor_correction_Float = sensor_correction_Float - 1;
    sensor_correction_String = String(sensor_correction_Float);
    sensor_correction_String.toCharArray(data1, 100);

    SPIFFS_write("/test1.txt", data1);
  }

  if (Request3){
    Serial.println("Interrupt Request Received! 3333333333333");
    Request3 = false;
    sensor_correction_Float2 = SPIFFS_read_float("/test2.txt");
    sensor_correction_Float2 = sensor_correction_Float2 + 1;
    sensor_correction_String2 = String(sensor_correction_Float2);
    sensor_correction_String2.toCharArray(data2, 100);

    SPIFFS_write("/test2.txt", data2);
  }

  if (Request4){
    Serial.println("Interrupt Request Received! 44444444444444");
    Request4 = false;
    sensor_correction_Float2 = SPIFFS_read_float("/test2.txt");
    sensor_correction_Float2 = sensor_correction_Float2 - 1;
    sensor_correction_String2 = String(sensor_correction_Float2);
    sensor_correction_String2.toCharArray(data2, 100);
    
    SPIFFS_write("/test2.txt", data2);
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
  // check_sensor_type();
  checkButton();
  if(is_AHT == true){
    task_temp();
  }

  if(is_count_30s == true)
  {
    Serial.println("t2 30s yes ======================================================~~~~!!!");
    task_co2(topic_sn, topic_sn2);
  // task_co(topic_sn, topic_sn2);
  // task_pm25(topic_sn, topic_sn2);
  // task_ch2o(topic_sn, topic_sn2)
  }
  
  Serial.println("t2 ======================");
}

void t3Callback() {
  int result = 0;

  result = myFunction(5, 8);

  delay(2000);
  Serial.printf("myFunction ==>> %d \n", result);
  Serial.println("t3 ======================");
}

void t4Callback() {
  checkButton();
  // Serial.println("t4 ======================");
}

void t5Callback() {
  count_30 = count_30 + 1;

  if (count_30 > 30)
  {
    Serial.println("t5 count_30 > 30====================== count_30 > 30 ");
    is_count_30s = true;
  }

  Serial.println("t5 ======================");
}

Task t1(1000, TASK_FOREVER, &t1Callback);
Task t2(1000, TASK_FOREVER, &t2Callback);
Task t3(5000, TASK_FOREVER, &t3Callback);
Task t4(100, TASK_FOREVER, &t4Callback);
Task t5(1000, TASK_FOREVER, &t5Callback);

void task_setup() {
  runner.init();
  runner.addTask(t1);
  runner.addTask(t2);
  runner.addTask(t3);
  runner.addTask(t4);
  runner.addTask(t5);

  t1.enable();
  t2.enable();
  // t3.enable();
  t4.enable();
  t5.enable();
}
// Task Function END of Line
// ==================================================

// ==================================================
// System Setup
// ==================================================
void setup () {
  Serial.begin(115200);
  SPIFFS_begin();
  QA_mode();
  Wifi_Setup(); // Wifi html keyin id/pwd
  AHT20_Setup();
  setup_isr();
  read_sensor_sn();
  serial_monitor();

  // delay(60000); // Delay for sensor initaliz time
  task_setup(); // Run Task
}

// ==================================================
// System Main Loop
// ==================================================
void loop () {
  // checkButton();
  runner.execute();
  // delay(50);
}