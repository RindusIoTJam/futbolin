#pragma once

#include <Arduino.h>
#define ENABLE_GxEPD2_GFX 0
#include "SPIFFS.h"
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <FS.h>
#include <iostream>
#define FileClass fs::File

class futbolin_display
{
private:
    /* data */
    static const uint16_t input_buffer_pixels = 640; // may affect performance
    static const uint16_t max_row_width = 640; // for up to 7.5" display
    static const uint16_t max_palette_pixels = 256; // for depth <= 8

    GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT> *display;

    uint8_t input_buffer[3 * input_buffer_pixels]; // up to depth 24
    uint8_t output_row_mono_buffer[max_row_width / 8]; // buffer for at least one row of b/w bits
    uint8_t output_row_color_buffer[max_row_width / 8]; // buffer for at least one row of color bits
    uint8_t mono_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 b/w
    uint8_t color_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 c/w

    std::string last_green_player_up;
    std::string last_green_player_down;
    std::string last_blue_player_up;
    std::string last_blue_player_down;

    int last_green_score;
    int last_blue_score;

    void drawBitmapFromSpiffs(const char *filename, int16_t x, int16_t y, bool with_color = true);
    void drawBitmapFromSpiffs_Buffered(const char *filename, int16_t x, int16_t y, bool with_color, bool partial_update, bool overwrite);
    uint16_t read16(fs::File& f);
    uint32_t read32(fs::File& f);

public:
    futbolin_display(/* args */);
    ~futbolin_display();
    void display_scoreboard(void);
    void update_green_players(std::string player_up,std::string player_down);
    void update_blue_players(std::string player_up,std::string player_down);
    void update_green_score(int score);
    void update_blue_score(int score);

};

