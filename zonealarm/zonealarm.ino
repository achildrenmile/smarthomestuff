#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include "secrets.h"

WiFiClient mainESP;
PubSubClient MQTT(mainESP);


int pirPin = 16;  //set the GPIO which you will connect the PIR sensor
bool lowPower = false; //set to true if you want low power use, slower alerts but more battery
int delayTime = 2000; //ONLY FOR LOW POWER - how long motion detected should be active

int pir=0;

void setup() {
  strcat(mqtt_maintopic, mqtt_subtopic);
  pinMode(pirPin, INPUT);
  Serial.begin(115200);
}

void loop() {
  if (!lowPower && WiFi.status() != WL_CONNECTED) startWiFi();
  Serial.println("startloop");
  
  pir=digitalRead(pirPin);
  Serial.println(pir);

  if (pir == LOW) delay(250);

  else {
    Serial.println("motion detected");
    if (WiFi.status() != WL_CONNECTED) startWiFi();
    MQTT.loop();
    if (!MQTT.connected()) reconnect();

    while (digitalRead(pirPin) == HIGH) {
      if (!MQTT.connected()) reconnect();
      MQTT.publish(mqtt_maintopic, "TRUE");
      Serial.println("Message Published: TRUE");
      delay(10000); //check all 10 seconds once one motion is detected
    }
    
    if (!MQTT.connected()) reconnect();
    if (lowPower) delay(delayTime);
    MQTT.publish(mqtt_maintopic, "FALSE");
    Serial.println("Message Published: FALSE");
    MQTT.loop();
    delay(500); //check all 0.5 seconds once no motion is detected
  }
  if (lowPower) {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);
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
  WiFi.hostname(mqtt_name);
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  MQTT.setServer(mqtt_server, 1883);
}
