#include <Arduino.h>
#include <iostream>
#include "Display/futbolin_display.hpp"

class futbolin_display *futbolin_scoreboard;

void setup()
{
  Serial.begin(115200);
  futbolin_scoreboard = new futbolin_display();
  Serial.println();
  Serial.println("setup");
  futbolin_scoreboard->display_scoreboard();
  Serial.println("setup done");
}

const char names[4][8] = { {"Francho"},{"Lidia"},{"Javi"},{"Enrique"}};
void loop()
{
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
        futbolin_scoreboard->update_green_players(std::string(names[i]),std::string(names[j]));
        futbolin_scoreboard->update_blue_players(std::string(names[j]),std::string(names[i]));
        futbolin_scoreboard->update_green_score(i);
        futbolin_scoreboard->update_blue_score(j);
        delay(5000);
    }
    delay(5000);
  }

}
