#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"

#define SS_PIN 15
#define RST_PIN 0

MFRC522 mfrc522(SS_PIN, RST_PIN);

WiFiClient mainESP;
PubSubClient MQTT(mainESP);

unsigned long cardId = 0;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  MQTT.setServer(mqtt_server, 1883);
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
void loop() {
  reconnect();

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  cardId = getCardId();

  Serial.print("New ");
  Serial.println(cardId);

  char uidbuffer[10];
  sprintf(uidbuffer, "%lu", cardId);

  StaticJsonDocument<100> rfiddoc;
   
  rfiddoc["sensorType"] = "RFID";
  rfiddoc["UID"]=uidbuffer;
  rfiddoc["State"]="new";


  char buffer[100];

  serializeJson(rfiddoc, buffer);
  
  MQTT.publish(mqtt_topic, buffer);

  uint8_t control = 0x00;

  do {
    control = 0;
    for (int i = 0; i < 3; i++) {
      if (!mfrc522.PICC_IsNewCardPresent()) {
        if (mfrc522.PICC_ReadCardSerial()) {
          control |= 0x16;
        }

        if (mfrc522.PICC_ReadCardSerial()) {
          control |= 0x16;
        }

        control += 0x1;
      }

      control += 0x4;
    }

    delay(10);
  } while (control == 13 || control == 14);

  reconnect();

  Serial.println("Removed");

  rfiddoc["State"]="removed";
  serializeJson(rfiddoc, buffer);
  
  MQTT.publish(mqtt_topic, buffer);
  delay(500);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

unsigned long getCardId() {
  byte readCard[4];
  for (int i = 0; i < 4; i++) {
    readCard[i] = mfrc522.uid.uidByte[i];
  }

  return (unsigned long)readCard[0] << 24
    | (unsigned long)readCard[1] << 16
    | (unsigned long)readCard[2] << 8
    | (unsigned long)readCard[3];
}
