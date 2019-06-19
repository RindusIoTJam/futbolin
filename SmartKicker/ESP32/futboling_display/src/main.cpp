#include <Arduino.h>
#include <LoRa.h>
#include <iostream>
#include "Display/futbolin_display.hpp"
#include <SPI.h>
#include <string.h>

#define ss 5
#define rst 22
#define dio0 2

class futbolin_display *futbolin_scoreboard;
class SPIClass LoraSPI(VSPI);

char last_received;
int counter_green = 0;
int counter_blue = 0;

void onReceive(int packetSize);

void setup()
{
  Serial.begin(115200);
  futbolin_scoreboard = new futbolin_display(true);
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

const char names[4][8] = { {"Francho"},{"Lidia"},{"Javi"},{"Enrique"}};
futbolin_display::futbolin_team_t test_teamA,test_teamB;

strcpy(test_teamA.player_down,names[0]);
strcpy(test_teamA.player_up,names[1]);
strcpy(test_teamB.player_up,names[2]);
strcpy(test_teamB.player_down,names[3]);

futbolin_scoreboard->set_blue_team(test_teamA);
futbolin_scoreboard->set_green_team(test_teamB);

}




void loop()
{

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