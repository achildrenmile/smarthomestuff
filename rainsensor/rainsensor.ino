#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

#define rainAnalog A0

WiFiClient mainESP;
PubSubClient MQTT(mainESP);

const int sensorMin = 0;     // sensor minimum
const int sensorMax = 1024;  // sensor maximum

bool lowPower = false; //set to true if you want low power use, slower alerts but more battery

int rained=false;

void setup() {
  strcat(mqtt_maintopic, mqtt_subtopic);
  Serial.begin(115200);
}

void loop() {
  StaticJsonDocument<100> rainsensordoc;
   
  rainsensordoc["sensorType"] = "RainSensor";
 
  char buffer[100];
    
  if (!lowPower && WiFi.status() != WL_CONNECTED) startWiFi();

  int sensorReading=analogRead(rainAnalog);
  //Serial.println(sensorReading);

  int range = map(sensorReading, sensorMin, sensorMax, 0, 3);

  if (range >= 2) 
  {
    if(rained)
    {//if rain stops
        if (WiFi.status() != WL_CONNECTED) startWiFi();
        MQTT.loop();
        rainsensordoc["raintype"]="Rain stopped";
        rainsensordoc["analogValue"]=0;
        serializeJson(rainsensordoc, buffer);
        MQTT.publish(mqtt_maintopic, buffer);
        Serial.println("Message Published: ");
        Serial.println(buffer);
        if (!MQTT.connected()) reconnect();
        rained=false;
    }
    
    delay(250);
        
  }
  else {
    rained=true;
    if (WiFi.status() != WL_CONNECTED) startWiFi();
    MQTT.loop();
    if (!MQTT.connected()) reconnect();
    
    

    Serial.println("Range:");
    Serial.println(range);

    switch (range) 
    {
          case 0:    
            rainsensordoc["raintype"]="Flood";
            break;
          case 1:   
            rainsensordoc["raintype"]="Rain Warning";
            break;
    }

      rainsensordoc["analogValue"]=sensorReading;

      serializeJson(rainsensordoc, buffer);
      MQTT.publish(mqtt_maintopic, buffer);
      Serial.println("Message Published: ");
      Serial.println(buffer);
      delay(10000); //check all 100 seconds once rain was detected
  }
  
  if (lowPower) {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!MQTT.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (MQTT.connect(mqtt_name)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(MQTT.state());
      Serial.println(" try again in 5 seconds");
      for (int i = 0; i < 5000; i++) {
        delay(1);
      }
    }
  }
}

void startWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }
  WiFi.hostname(mqtt_name);
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  MQTT.setServer(mqtt_server, 1883);
  MQTT.setCallback(callback);
}
