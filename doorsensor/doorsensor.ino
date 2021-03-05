#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include "secrets.h"
#include <ArduinoJson.h>

WiFiClient mainESP;
PubSubClient MQTT(mainESP);


int switchPin = 16;

int reedswitch=0;

void setup() {
  pinMode(switchPin, INPUT_PULLUP);
  Serial.begin(115200);
}

void loop() {
  
  StaticJsonDocument<100> doorsensordoc;
  char buffer[100];
  
  if (WiFi.status() != WL_CONNECTED) startWiFi();
  Serial.println("startloop");
  
  reedswitch=digitalRead(switchPin);
  Serial.println(reedswitch);

  if (reedswitch == LOW) delay(250);

  else {
    Serial.println("door open detected");
    if (WiFi.status() != WL_CONNECTED) startWiFi();
    MQTT.loop();
    if (!MQTT.connected()) reconnect();

    while (digitalRead(switchPin) == HIGH) {
      if (!MQTT.connected()) reconnect();

      doorsensordoc["open"]=1;
      doorsensordoc["humanmessage"]="is open";
      serializeJson(doorsensordoc, buffer);
      MQTT.publish(mqtt_topic, buffer);
      Serial.println("Message Published:");
      Serial.println(buffer);
      delay(10000);
    }
    
    if (!MQTT.connected()) reconnect();

    doorsensordoc["open"]=0;
    doorsensordoc["humanmessage"]="is closed";
    serializeJson(doorsensordoc, buffer);
    MQTT.publish(mqtt_topic, buffer);
    
    Serial.println("Message Published:");
    Serial.println(buffer);
    MQTT.loop();
    delay(500); 
  }
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
  WiFi.setHostname(mqtt_name);
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  MQTT.setServer(mqtt_server, 1883);
}
