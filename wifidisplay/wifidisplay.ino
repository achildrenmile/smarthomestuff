#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include <PubSubClient.h>
#include "secrets.h"
#include <ArduinoJson.h>

WiFiClient mainESP;
PubSubClient client(mainESP);

long lastMsg = 0;
char msg[50];
int value = 0;

SSD1306 display(0x3c, D2, D1); //NodeMCU

#define D1mini 1

void setup() {
  
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  reconnect();
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  display.clear();
  String msg;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    //display.clear();
    msg += (char)payload[i];
  }

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, msg);

  String temperature="t: " + String((double)doc["temperature"]) + " °C ";
  String humidity="h: " + String((double)doc["humidity"]) + " %";
  
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(35,13,"Office");
  display.drawString(35,26,temperature);
  display.drawString(35,36,humidity); 
  display.drawString(35,46,doc["aiq"]); 
  display.display();
 
  Serial.println();

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    Serial.println(devicename);
    if (client.connect(devicename)) {
      Serial.println("connected");
      client.subscribe("homeassistant/environmentzone1/data");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

 // if (!client.connected()) {
  //  reconnect();
 // }
  client.loop();
  
}  
