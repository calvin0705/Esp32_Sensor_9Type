#include <WiFi.h>


const char* ssid = "tfa00";
const char* password = "tfa00tfa00";

int wifi_status;

void setup_wifi() {

  wifi_status = WiFi.begin(ssid, password);
  Serial.printf("wifi_status1111 ===========>>> %d \n ", wifi_status);

  int c=0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    Serial.println("A");
    c=c+1;
    if(c>10){
        ESP.restart();
    }
  }

  Serial.printf("wifi_status2222 =========================>>> %d \n ", WiFi.status());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  setup_wifi();
}

void loop() 
{
}
