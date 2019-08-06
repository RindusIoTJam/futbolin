#include <Arduino.h>

#include <LoRa.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <AutoConnect.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include "webpage.hpp"
#include "DNSServer.h"

#define debug 1

#ifndef debug
#define debug_uart(baudrate)
#define debug_tracesln(trace)
#define debug_traces(trace)
#define debug_tracestype(trace,type)
#else
#define debug_uart(baudrate) Serial.begin(baudrate)
#define debug_tracesln(trace) Serial.println(trace)
#define debug_traces(trace) Serial.print(trace)
#define debug_tracestype(trace,type) Serial.print(trace,type)
#endif

#define AUTOCONNECT_APID "IoTFutbolin"
#define AUTOCONNECT_PSK "RindusIoTFutbolin"

#define ss 5
#define rst 22
#define dio0 2

const char* host = AUTOCONNECT_APID;
const char* ap = AUTOCONNECT_APID;
const char* password = AUTOCONNECT_PSK;
const char* update_path = "/firmware";
const char* update_username = AUTOCONNECT_APID;
const char* update_password = AUTOCONNECT_PSK;
const char* mqttServer = "mqtt.rindus.es";
const int mqttPort = 8883;
const char* mqttUser = "futbolin";
const char* mqttPassword = "IqucaEquna785";
const char* gatewayTopic = "rindus/futbolin/events";


class WiFiClientSecure espClient;
class PubSubClient client(espClient);
class WebServer Server;
class AutoConnect Portal(Server);
class SPIClass LoraSPI(VSPI);
//class DNSServer dnsServer;

void onReceive(int packetSize);
void connectMQTTServer();
void MQTTcallback(char* topic, byte* payload, unsigned int length);

void rootPage();
void setupWebPage();


QueueHandle_t LoRaEvents_Queue = xQueueCreate( 32, sizeof(char));

void setup() 
{
  // put your setup code here, to run once:
  debug_uart(115200);

  debug_tracesln("Setting web page");
  setupWebPage();
  debug_tracesln("Web page setted");

  Serial.printf("[OTA] HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);
  debug_tracesln("[OTA] OTA Ready");

  debug_tracesln("Starting LoRa 433E6");
  LoraSPI.begin();
  LoRa.setSPI(LoraSPI);
  LoRa.setPins(ss,rst,dio0);

  if (!LoRa.begin(433E6))
    while (1);

  debug_tracesln("LoRa setup correctly");

  LoRa.onReceive(onReceive);
  LoRa.receive();

  if (Portal.begin())
  {
    debug_tracesln("WiFi connected: " + WiFi.localIP().toString());
  }
  else
  {
    debug_tracesln("Mode AP iniciated");
  }

  //dnsServer.start(53, "*", WiFi.localIP());
  connectMQTTServer();
  
}

void loop()
{
  // put your main code here, to run repeatedly:
  Portal.handleClient();

  if (uxQueueMessagesWaiting(LoRaEvents_Queue) > 0)
  {
    char LoRaEvent;
    xQueueReceive(LoRaEvents_Queue,&LoRaEvent,1);
    debug_traces("\nLoRa received event: ");
    debug_tracesln(LoRaEvent);
    client.publish(gatewayTopic,&LoRaEvent);
  }

  client.loop();

}

void onReceive(int packetSize) 
{
  char Received = LoRa.read();
  xQueueSendFromISR(LoRaEvents_Queue,&Received,pdFALSE);
}

void connectMQTTServer()
{
  client.setServer(mqttServer, mqttPort);
 
  while (!client.connected()) {
    debug_tracesln("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
      debug_tracesln("Connected to MQTT server");  
    } else {
     debug_traces("Failed connection to MQTT Server");
      debug_tracesln(client.state());
      delay(2000);
    }
  }
}



void setupWebPage()
{
    Portal.config(host,password);
    Server.on("/Menu", rootPage);

    Server.on("/", HTTP_GET, []() {
    Server.sendHeader("Connection", "close");
    Server.send(200, "text/html", loginIndex);
  });
  Server.on("/ServerIndex", HTTP_GET, []() {
    Server.sendHeader("Connection", "close");
    Server.send(200, "text/html", ServerIndex);
  });
  /*handling uploading firmware file */
  Server.on("/update", HTTP_POST, []() {
    Server.sendHeader("Connection", "close");
    Server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = Server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
}

 void rootPage() 
{
  const char content[] = "<html> <head><style>body {background-color: gray;}h1   {color: blue;}p    {color: white;}</style></head><body><h1>Rindus IoT Futbolin</h1><p>Configure the WIFI settings by clicking in the following link: <a href=\"/_ac\">Wifi settings</a> <br/> Flash firmware by clicking in the following link: <a href=\"/ServerIndex\">Flash firmware</a> </p></body></html>";
  Server.send(200, "text/html", content);
}