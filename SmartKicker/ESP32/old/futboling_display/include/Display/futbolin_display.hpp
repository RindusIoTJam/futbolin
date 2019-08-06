#pragma once

#include <Arduino.h>
#include <FreeRTOS.h>
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
public:
    typedef struct
{
    char player_up[20];
    char player_down[20];
} futbolin_team_t;
private:
    /* data */
    bool _async;

    static const uint16_t input_buffer_pixels = 640; // may affect performance
    static const uint16_t max_row_width = 640; // for up to 7.5" display
    static const uint16_t max_palette_pixels = 256; // for depth <= 8

    GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT> *display;

    uint8_t input_buffer[3 * input_buffer_pixels]; // up to depth 24
    uint8_t output_row_mono_buffer[max_row_width / 8]; // buffer for at least one row of b/w bits
    uint8_t output_row_color_buffer[max_row_width / 8]; // buffer for at least one row of color bits
    uint8_t mono_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 b/w
    uint8_t color_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 c/w

    futbolin_team_t last_green_team;
    futbolin_team_t last_blue_team;

    int last_green_score;
    int last_blue_score;

    uint16_t read16(fs::File& f);
    uint32_t read32(fs::File& f);

    void drawBitmapFromSpiffs(const char *filename, int16_t x, int16_t y, bool with_color = true);
    void drawBitmapFromSpiffs_Buffered(const char *filename, int16_t x, int16_t y, bool with_color, bool partial_update, bool overwrite);
  
    BaseType_t xReturnedScoreboard;
    TaskHandle_t xTask_scoreboard = NULL;

    BaseType_t xReturnedgreen_team;
    TaskHandle_t xTask_green_team = NULL;

    BaseType_t xReturnedblue_team;
    TaskHandle_t xTask_blue_team = NULL;

    BaseType_t xReturnedgreen_score;
    TaskHandle_t xTask_green_score = NULL;

    BaseType_t xReturnedblue_score;
    TaskHandle_t xTask_blue_score = NULL;

    QueueHandle_t xblue_score_Queue = NULL;
    QueueHandle_t xgreen_score_Queue = NULL;
    QueueHandle_t xblue_team_Queue = NULL;
    QueueHandle_t xgreen_team_Queue = NULL;

    SemaphoreHandle_t xdisplay_mutex = NULL;
    SemaphoreHandle_t xdisplay_scoreboard_semaphore = NULL;

    static void display_scoreboard_async(void*);
    static void update_blue_score_async(void*);
    static void update_green_score_async(void*);
    static void update_blue_team_async(void*);
    static void update_green_team_async(void*);


public:
    futbolin_display(bool async = false);
    ~futbolin_display();

    void display_scoreboard(void);
    void update_green_players(futbolin_team_t player);
    void update_blue_players(futbolin_team_t player);
    void update_green_score(int score);
    void update_blue_score(int score);
    void set_blue_score(int score);
    void set_green_score(int score);
    void set_scoreboard(void);
    void set_blue_team(futbolin_team_t player);
    void set_green_team(futbolin_team_t player);



};

