#include <Servo.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>

const char* ssid     = "";
const char* password = "";
char* mqtt_server = ""; /
char* mqtt_name = "Mailbox Servo 1"; //MQTT device name
char* mqtt_topic = "homeassistant/mailbox1/set"; //MQTT device name

Servo Servo_SG90;
ESP8266WebServer WebServer ( 80 );

WiFiClient mainESP;
PubSubClient MQTT(mainESP);

int SERVOCurrent = 0;

void setup()
{
  // Wir debuggen auf die serielle Schnittstelle (Seriellen Monitor im der Adruino-Programm starten!)
  Serial.begin(115200);
  delay(200);
  Serial.println("Verbinde Servo.");
  Servo_SG90.attach(D5);
  Serial.println("Set servo to zero.");
  Servo_SG90.write(0);
  startWiFi();
  beginServer();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) startWiFi();
  MQTT.loop();
  if (!MQTT.connected()) reconnect();
  
  WebServer.handleClient();
  
   
}


void callback(char* topic, byte* payload, unsigned int length) {
  if(SERVOCurrent==90)
    SERVOCurrent=0;
  else
    SERVOCurrent=90;
    
   Servo_SG90.write(SERVOCurrent);
}

void reconnect() {
  while (!MQTT.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (MQTT.connect(mqtt_name)) {
      Serial.println("connected");
      MQTT.subscribe(mqtt_topic);
      Serial.println("mqtt subscribed");
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

void beginServer()
{
  WebServer.on ( "/", handleRoot ); // Wenn die URL aufgerufen wird (ohne etwas hinter dem /) wird die Funktion "handleRoot" gestartet
  WebServer.begin();
  Serial.println ( "HTTP Webserver started!" );
}

// Wird von der Funktion "beginServer" aufgerufen
void handleRoot() {
  if ( WebServer.hasArg("SERVO") ) { // Wir wurden mit dem Argument "SERVO" aufgerufen - es gibt also gleich etwas zu tun!
    handleSubmit();
  } else {
    // Einfach nur die Webseite ausliefern
    WebServer.send ( 200, "text/html", getPage() );
  }
}

// jemand hat auf der Webseite eine der Schaltflächen / Buttons gedrückt
void handleSubmit() {
  // Wir holen uns den übermittelten Wert
  String SERVOValue;
  SERVOValue = WebServer.arg("SERVO");
  SERVOCurrent = SERVOValue.toInt();

  // Jetzt können wir den Wert setzen:
  Serial.print("Set angle ");
  Serial.print(SERVOCurrent);
  Serial.println(" degree.");
  Servo_SG90.write(SERVOCurrent);
  WebServer.send ( 200, "text/html", getPage() );
}

String getPage() {
  String ServoString = String(SERVOCurrent); // Holt die aktuelle Einstellung und wandelt diese in einen String. Ist untem im Code eingebaut.
  String page = "<html lang=de-DE><head>";
  //page += "<meta http-equiv='refresh' content='60'/>";
  page += "<title>Wemos D1 Mini + Servo SG90 Test</title>";
  page += "<style> body { background-color: #fffff; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }";
  page += ".button {display: inline-block; padding: 15px 15px; font-size: 25px; cursor: pointer; text-align: center; text-decoration: none;";
  page += "outline: none; color: #ffffff; background-color: #4db2ec; border: none; border-radius: 5px;}";
  page += ".button:hover {background-color: #4DCAEC; color:#ffffff;}";
  page += ".button:active {background-color: #4DCAEC; box-shadow: 0 3px #666; transform: translateY(3px); }</style>";
  page += "</head><body><center><h1>Wemos D1 Mini - Servo SG90 - WebServer</h1>";
  page += "<hr><h2>aktueller Wert: " + ServoString + " Grad</h1><hr>";
  page += "<h3>Feste Werte</h3>";
  page += "<form action='/' method='POST'>";
  page += "<INPUT class='button' type='submit' name='SERVO' value='0'>       ";
  page += "<INPUT class='button' type='submit' name='SERVO' value='90'>";
  page += "<h3>+/-</h3>";
  page += "<form action='/' method='POST'>";
  page += "</center></body>";
  return page;
}
