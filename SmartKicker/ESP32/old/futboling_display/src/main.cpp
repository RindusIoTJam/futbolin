#include <Arduino.h>
#include <LoRa.h>
#include <iostream>
#include "Display/futbolin_display.hpp"
#include <SPI.h>
#include <string.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <AutoConnect.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>

#define ss 5
#define rst 22
#define dio0 2

#define AUTOCONNECT_APID "IoTFutbolin"
#define AUTOCONNECT_PSK "RindusIoTFutbolin"

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

const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>Rindus Futbolin Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='IoTFutbolin' && form.pwd.value=='RindusIoTFutbolin')"
    "{"
    "window.open('/Menu')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";
 
/*
 * Server Index Page
 */
 
const char* ServerIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

class futbolin_display *futbolin_scoreboard;
class SPIClass LoraSPI(VSPI);
class WebServer Server;
class AutoConnect Portal(Server);
class WiFiClientSecure espClient;
class PubSubClient client(espClient);


void connectMQTTServer();
void MQTTcallback(char* topic, byte* payload, unsigned int length);
void parseMsgToQueue(byte* mes, unsigned int l);
void rootPage();

char last_received;
int counter_green = 0;
int counter_blue = 0;
QueueHandle_t team_Queue = xQueueCreate( 10, sizeof( futbolin_display::futbolin_team_t ));

void onReceive(int packetSize);

void setup()
{
  Serial.begin(115200);
  futbolin_scoreboard = new futbolin_display(true);

  Portal.config(AUTOCONNECT_APID,AUTOCONNECT_PSK);
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

  Serial.printf("[OTA] HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);
  Serial.println("[OTA] OTA Ready");

  Serial.println("Setup finish");

  Serial.println();
  Serial.println("setup Display");
  futbolin_scoreboard->set_scoreboard();
  Serial.println("setup done");
  Serial.println("Setup Lora");
  LoraSPI.begin();
  LoRa.setSPI(LoraSPI);
  LoRa.setPins(ss,rst,dio0);
  if (!LoRa.begin(433E6))
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.onReceive(onReceive);
  LoRa.receive();

  Serial.println("setup done");

  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
    //wifiServer.begin();
    //Serial.println("[door Server] Command Server ready");
  }
  else
  {
    Serial.println("Mode AP iniciated");
  }
  if (!MDNS.begin("iotfutbolin")) {
        Serial.println("Error setting up MDNS responder!");
        while(1) {
            delay(1000);
        }
    }
  Serial.println("mDNS responder started");
  MDNS.addService("_http", "_tcp", 80);

  Serial.println("Connecting MQTT");
  connectMQTTServer();
  client.subscribe("rindus/futbolin");
  
  Serial.println("Wait for two teams");

  while (uxQueueMessagesWaiting(team_Queue) < 2)
  {
    vTaskDelay(10/portTICK_PERIOD_MS); //give time for other tasks
    loop();
  }

  futbolin_display::futbolin_team_t team_received;
  xQueueReceive(team_Queue,&team_received,portMAX_DELAY);
  futbolin_scoreboard->set_blue_team(team_received);
  xQueueReceive(team_Queue,&team_received,portMAX_DELAY);
  futbolin_scoreboard->set_green_team(team_received);
}




void loop()
{
  Portal.handleClient();
  client.loop();
}

void onReceive(int packetSize) {
  // received a packet
  Serial.print("Received packet '");

  // read packet
    char Received = LoRa.read();
    Serial.print(Received);

 // print RSSI of packet
  Serial.print("' with RSSI ");
  Serial.println(LoRa.packetRssi());

  if (Received == 'r')
  {
    counter_blue++;
    futbolin_scoreboard->set_blue_score(counter_blue);
  }
  else if (Received == 'l')
  {
    counter_green++;
    futbolin_scoreboard->set_green_score(counter_green);
  }
  else if (Received == 'c')
  {
    if (uxQueueMessagesWaiting(team_Queue) > 0)
    {
      futbolin_display::futbolin_team_t team_received;
      xQueueReceive(team_Queue,&team_received,0);
      if (counter_blue < counter_green)
      {
        futbolin_scoreboard->set_blue_team(team_received);
      }
      else
      {
        futbolin_scoreboard->set_green_team(team_received);
      }
      
    }
    counter_blue = 0;
    counter_green = 0;
    futbolin_scoreboard->set_green_score(counter_green);
    futbolin_scoreboard->set_blue_score(counter_blue);
  }
  else if (Received == 'n')
  {
    if (last_received == 'l')
    {
    counter_green--;
    futbolin_scoreboard->set_green_score(counter_green);
    }
     if (last_received == 'r')
    {
    counter_blue--;
    futbolin_scoreboard->set_blue_score(counter_blue);
    }
    
  }
  last_received = Received;
  
  Serial.print("counter_blue: ");
  Serial.printf("%d \n",counter_blue);
  Serial.print("counter_green: ");
  Serial.printf("%d \n",counter_green);
}

void MQTTcallback(char* topic, byte* payload, unsigned int length) {
 //#ifdef DEBUG
 //{
  Serial.println("-----------------------");
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
 //}
 parseMsgToQueue(payload, length);
}

void connectMQTTServer()
{
  client.setServer(mqttServer, mqttPort);
  client.setCallback(MQTTcallback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
      Serial.println("Connected to MQTT server");  
    } else {
      Serial.print("Failed connection to MQTT Server");
      Serial.print(client.state());
      delay(2000);
    }
  }
}

void parseMsgToQueue(byte* mes, unsigned int lon)
{
  mes = mes + 1;
  
  //#ifdef DEBUG
  //{
    String s1;
    for (int i = 0; i < lon-1; i++) {
      s1=s1+(char)mes[i];
    }
    String name1 = s1.substring(0,s1.indexOf(' ')); //Desde el primer caracter, hasta el primer espacio
    Serial.print("NAME 1: ");
    Serial.println(name1);
    String name2 = s1.substring(s1.indexOf('/')+1);
    name2 = name2.substring(0,name2.indexOf(' ')); //Desde el siguiente caracter a '/' hasta el siguiente espacio
    Serial.print("NAME 2: ");
    Serial.println(name2);  
  //}
    futbolin_display::futbolin_team_t couple;
    strcpy(couple.player_up,name1.c_str());
    strcpy(couple.player_down,name2.c_str());
    xQueueSend(team_Queue,&couple,1);

    //LLAMADA DE PRUEBA A PUBLICAR COLA
   // if (name2[0] == 'J')
    //{
     // getQueue();
    //}
}

void rootPage() 
{
  const char content[] = "<html> <head><style>body {background-color: gray;}h1   {color: blue;}p    {color: white;}</style></head><body><h1>Rindus IoT Futbolin</h1><p>Configure the WIFI settings by clicking in the following link: <a href=\"/_ac\">Wifi settings</a> <br/> Flash firmware by clicking in the following link: <a href=\"/ServerIndex\">Flash firmware</a> </p></body></html>";
  Server.send(200, "text/html", content);
}