#include "../include/Display/futbolin_display.hpp"


futbolin_display::futbolin_display(bool async)
{
  _async = async;

  static GxEPD2_3C<GxEPD2_750c, GxEPD2_750c::HEIGHT> display_init(GxEPD2_750c(/*CS=*/ 15, /*DC=*/ 27, /*RST=*/ 26, /*BUSY=*/ 25));
  this->display = &display_init;
  display->init(115200);
  SPI.end();
  SPI.begin(13, 12, 14, 15);
  SPIFFS.begin();

  last_blue_team.player_down[0] = '\0';
  last_blue_team.player_up[0] = '\0';
  last_green_team.player_down[0] = '\0';
  last_green_team.player_up[0] = '\0';

  last_green_score = 0;
  last_blue_score = 0;

  if (async)
  {
    xblue_score_Queue = xQueueCreate( 5, sizeof( int ) );
    xgreen_score_Queue = xQueueCreate( 5, sizeof( int ) );

    xblue_team_Queue = xQueueCreate( 10, sizeof( futbolin_display::futbolin_team_t ));
    xgreen_team_Queue = xQueueCreate( 10, sizeof( futbolin_display::futbolin_team_t ));

    xdisplay_mutex = xSemaphoreCreateMutex();
    xdisplay_scoreboard_semaphore = xSemaphoreCreateBinary();
    xSemaphoreTake(xdisplay_scoreboard_semaphore,0);

    xReturnedgreen_score = xTaskCreate(
                    this->update_green_score_async,       /* Function that implements the task. */
                    "green_score",   /* Text name for the task. */
                    CONFIG_FREERTOS_IDLE_TASK_STACKSIZE*2,      /* Stack size in words, not bytes. */
                    this,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY+1,/* Priority at which the task is created. */
                    &xTask_green_score ); 

    xReturnedblue_score = xTaskCreate(
                    this->update_blue_score_async,       /* Function that implements the task. */
                    "blue_score",   /* Text name for the task. */
                    CONFIG_FREERTOS_IDLE_TASK_STACKSIZE*2,      /* Stack size in words, not bytes. */
                    this,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY+1,/* Priority at which the task is created. */
                    &xTask_blue_score ); 

    xReturnedScoreboard = xTaskCreate(
                  this->display_scoreboard_async,       /* Function that implements the task. */
                  "score_board",   /* Text name for the task. */
                  CONFIG_FREERTOS_IDLE_TASK_STACKSIZE*2,      /* Stack size in words, not bytes. */
                  this,    /* Parameter passed into the task. */
                  tskIDLE_PRIORITY+1,/* Priority at which the task is created. */
                  &xTask_scoreboard ); 

    xReturnedScoreboard = xTaskCreate(
                  this->update_green_team_async,       /* Function that implements the task. */
                  "green_team",   /* Text name for the task. */
                  CONFIG_FREERTOS_IDLE_TASK_STACKSIZE*2,      /* Stack size in words, not bytes. */
                  this,    /* Parameter passed into the task. */
                  tskIDLE_PRIORITY+1,/* Priority at which the task is created. */
                  &xTask_green_team ); 

    xReturnedScoreboard = xTaskCreate(
              this->update_blue_team_async,       /* Function that implements the task. */
              "blue_team",   /* Text name for the task. */
              CONFIG_FREERTOS_IDLE_TASK_STACKSIZE*2,      /* Stack size in words, not bytes. */
              this,    /* Parameter passed into the task. */
              tskIDLE_PRIORITY+1,/* Priority at which the task is created. */
              &xTask_blue_team );                

  }
  


}


futbolin_display::~futbolin_display()
{
    delete this->display;
}

void futbolin_display::set_scoreboard(void)
{
  if(_async)
  {
    xSemaphoreGive(xdisplay_scoreboard_semaphore);
  }
  else
  {
    this->display_scoreboard();
  }
  
}

void futbolin_display::display_scoreboard_async(void* _this)
{
  futbolin_display *ithis = reinterpret_cast<futbolin_display*> (_this);
  for(;;)
  {
    while(xSemaphoreTake(ithis->xdisplay_scoreboard_semaphore,portMAX_DELAY) != 1){}
    xSemaphoreTake(ithis->xdisplay_mutex,portMAX_DELAY);
    ithis->display_scoreboard();
    xSemaphoreGive(ithis->xdisplay_mutex);
  }
}

void futbolin_display::display_scoreboard(void)
{
  int16_t w2 = (display->width() - 640) / 2;
  int16_t h2 = (display->height() - 384) / 2;
  this->drawBitmapFromSpiffs_Buffered("VS1.bmp", w2, h2,true,true,false);
}

void futbolin_display::set_blue_team(futbolin_display::futbolin_team_t player)
{
  if(_async)
  {
    xQueueSend(xblue_team_Queue,&player,0);
  }
  else
  {
    this->update_blue_players(player);
  }
}

void futbolin_display::update_blue_team_async(void * _this)
{
  futbolin_display::futbolin_team_t blue_team;
  futbolin_display *ithis = reinterpret_cast<futbolin_display*> (_this);
  for(;;)
  {
    while(xQueueReceive(ithis->xblue_team_Queue,&blue_team,portMAX_DELAY) !=1){}
    xSemaphoreTake(ithis->xdisplay_mutex,portMAX_DELAY);
    ithis->update_blue_players(blue_team);
    xSemaphoreGive(ithis->xdisplay_mutex);
  }
}

void futbolin_display::update_blue_players(futbolin_display::futbolin_team_t player)
{
  display->setTextColor(GxEPD_YELLOW);
  display->setTextSize(2);
  display->setCursor(67,170);
  display->print(last_green_team.player_up);
  display->setCursor(67,200);
  display->print(last_green_team.player_down);
  display->setTextColor(GxEPD_WHITE);
  display->setTextSize(2);
  display->setCursor(67,170);
  display->print(player.player_up);
  display->setCursor(67,200);
  display->print(player.player_down);

  last_green_team = player;
  

  display->display(true);
}

void futbolin_display::set_green_team(futbolin_display::futbolin_team_t player)
{
  if(_async)
  {
    xQueueSend(xgreen_team_Queue,&player,0);
  }
  else
  {
    this->update_green_players(player);
  }
}

void futbolin_display::update_green_team_async(void * _this)
{
  futbolin_display::futbolin_team_t green_team;
  futbolin_display *ithis = reinterpret_cast<futbolin_display*> (_this);
  for(;;)
  {
    while(xQueueReceive(ithis->xgreen_team_Queue,&green_team,portMAX_DELAY) !=1){}
    xSemaphoreTake(ithis->xdisplay_mutex,portMAX_DELAY);
    ithis->update_green_players(green_team);
    xSemaphoreGive(ithis->xdisplay_mutex);
  }
}

void futbolin_display::update_green_players(futbolin_display::futbolin_team_t player)
{
    

    display->setTextColor(GxEPD_YELLOW);
    display->setTextSize(2);
    display->setCursor(460,170);
    display->print(last_blue_team.player_up);
    display->setCursor(460,200);
    display->print(last_blue_team.player_down);
    display->setTextColor(GxEPD_WHITE);
    display->setTextSize(2);
    display->setCursor(460,170);
    display->print(player.player_up);
    display->setCursor(460,200);
    display->print(player.player_down);

    last_blue_team    =   player;

    display->display(true);


}

void futbolin_display::set_blue_score(int score)
{
  if(_async)
  {
    xQueueSend(xblue_score_Queue,&score,0);
  }
  else
  {
    this->update_blue_score(score);
  }
  
}

void futbolin_display::update_blue_score_async(void* _this)
{
  int blue_score;
  futbolin_display *ithis = reinterpret_cast<futbolin_display*> (_this);
  for(;;)
  {
    while(xQueueReceive(ithis->xblue_score_Queue,&blue_score,portMAX_DELAY) !=1){}
    xSemaphoreTake(ithis->xdisplay_mutex,portMAX_DELAY);
    ithis->update_blue_score(blue_score);
    xSemaphoreGive(ithis->xdisplay_mutex);
  }
}

void futbolin_display::update_blue_score(int score)
{
    display->setTextColor(GxEPD_WHITE,GxEPD_COLORED);
    display->setTextSize(4);
    display->setCursor(210,240);
    display->printf("%d",score);

    display->display(true);
}

void futbolin_display::set_green_score(int score)
{
  if(_async)
  {
    xQueueSend(xgreen_score_Queue,&score,0);
  }
  else
  {
    this->update_green_score(score);
  }
  
}

void futbolin_display::update_green_score_async(void* _this)
{
  int green_score;
  futbolin_display *ithis = reinterpret_cast<futbolin_display*> (_this);
  for(;;)
  {
    while(xQueueReceive(ithis->xgreen_score_Queue,&green_score,portMAX_DELAY) !=1){}
    xSemaphoreTake(ithis->xdisplay_mutex,portMAX_DELAY);
    ithis->update_green_score(green_score);
    xSemaphoreGive(ithis->xdisplay_mutex);

  }
}

void futbolin_display::update_green_score(int score)
{
    display->setTextColor(GxEPD_WHITE,GxEPD_COLORED);
    display->setTextSize(4);
    display->setCursor(410,240);
    display->printf("%d",score);
    
    display->display(true);
}




void futbolin_display::drawBitmapFromSpiffs(const char *filename, int16_t x, int16_t y, bool with_color)
{
  fs::File file;
  bool valid = false; // valid format to be handled
  bool flip = true; // bitmap is stored bottom-to-top
  uint32_t startTime = millis();
  if ((x >= display->width()) || (y >= display->height())) return;
  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');
#if defined(ESP32)
  file = SPIFFS.open(String("/") + filename, "r");
#else
  file = SPIFFS.open(filename, "r");
#endif
  if (!file)
  {
    Serial.print("File not found");
    return;
  }
  // Parse BMP header
  if (read16(file) == 0x4D42) // BMP signature
  {
    uint32_t fileSize = read32(file);
    uint32_t creatorBytes = read32(file);
    uint32_t imageOffset = read32(file); // Start of image data
    uint32_t headerSize = read32(file);
    uint32_t width  = read32(file);
    uint32_t height = read32(file);
    uint16_t planes = read16(file);
    uint16_t depth = read16(file); // bits per pixel
    uint32_t format = read32(file);
    if ((planes == 1) && ((format == 0) || (format == 3))) // uncompressed is handled, 565 also
    {
      Serial.print("File size: "); Serial.println(fileSize);
      Serial.print("Image Offset: "); Serial.println(imageOffset);
      Serial.print("Header size: "); Serial.println(headerSize);
      Serial.print("Bit Depth: "); Serial.println(depth);
      Serial.print("Image size: ");
      Serial.print(width);
      Serial.print('x');
      Serial.println(height);
      // BMP rows are padded (if needed) to 4-byte boundary
      uint32_t rowSize = (width * depth / 8 + 3) & ~3;
      if (depth < 8) rowSize = ((width * depth + 8 - depth) / 8 + 3) & ~3;
      if (height < 0)
      {
        height = -height;
        flip = false;
      }
      uint16_t w = width;
      uint16_t h = height;
      if ((x + w - 1) >= display->width())  w = display->width()  - x;
      if ((y + h - 1) >= display->height()) h = display->height() - y;
      if (w <= max_row_width) // handle with direct drawing
      {
        valid = true;
        uint8_t bitmask = 0xFF;
        uint8_t bitshift = 8 - depth;
        uint16_t red, green, blue;
        bool whitish, colored;
        if (depth == 1) with_color = false;
        if (depth <= 8)
        {
          if (depth < 8) bitmask >>= depth;
          file.seek(54); //palette is always @ 54
          for (uint16_t pn = 0; pn < (1 << depth); pn++)
          {
            blue  = file.read();
            green = file.read();
            red   = file.read();
            file.read();
            whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
            colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
            if (0 == pn % 8) mono_palette_buffer[pn / 8] = 0;
            mono_palette_buffer[pn / 8] |= whitish << pn % 8;
            if (0 == pn % 8) color_palette_buffer[pn / 8] = 0;
            color_palette_buffer[pn / 8] |= colored << pn % 8;
          }
        }
        display->clearScreen();
        uint32_t rowPosition = flip ? imageOffset + (height - h) * rowSize : imageOffset;
        for (uint16_t row = 0; row < h; row++, rowPosition += rowSize) // for each line
        {
          uint32_t in_remain = rowSize;
          uint32_t in_idx = 0;
          uint32_t in_bytes = 0;
          uint8_t in_byte = 0; // for depth <= 8
          uint8_t in_bits = 0; // for depth <= 8
          uint8_t out_byte = 0xFF; // white (for w%8!=0 boarder)
          uint8_t out_color_byte = 0xFF; // white (for w%8!=0 boarder)
          uint32_t out_idx = 0;
          file.seek(rowPosition);
          for (uint16_t col = 0; col < w; col++) // for each pixel
          {
            // Time to read more pixel data?
            if (in_idx >= in_bytes) // ok, exact match for 24bit also (size IS multiple of 3)
            {
              in_bytes = file.read(input_buffer, in_remain > sizeof(input_buffer) ? sizeof(input_buffer) : in_remain);
              in_remain -= in_bytes;
              in_idx = 0;
            }
            switch (depth)
            {
              case 24:
                blue = input_buffer[in_idx++];
                green = input_buffer[in_idx++];
                red = input_buffer[in_idx++];
                whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
                colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                break;
              case 16:
                {
                  uint8_t lsb = input_buffer[in_idx++];
                  uint8_t msb = input_buffer[in_idx++];
                  if (format == 0) // 555
                  {
                    blue  = (lsb & 0x1F) << 3;
                    green = ((msb & 0x03) << 6) | ((lsb & 0xE0) >> 2);
                    red   = (msb & 0x7C) << 1;
                  }
                  else // 565
                  {
                    blue  = (lsb & 0x1F) << 3;
                    green = ((msb & 0x07) << 5) | ((lsb & 0xE0) >> 3);
                    red   = (msb & 0xF8);
                  }
                  whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
                  colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                }
                break;
              case 1:
              case 4:
              case 8:
                {
                  if (0 == in_bits)
                  {
                    in_byte = input_buffer[in_idx++];
                    in_bits = 8;
                  }
                  uint16_t pn = (in_byte >> bitshift) & bitmask;
                  whitish = mono_palette_buffer[pn / 8] & (0x1 << pn % 8);
                  colored = color_palette_buffer[pn / 8] & (0x1 << pn % 8);
                  in_byte <<= depth;
                  in_bits -= depth;
                }
                break;
            }
            if (whitish)
            {
              // keep white
            }
            else if (colored && with_color)
            {
              out_color_byte &= ~(0x80 >> col % 8); // colored
            }
            else
            {
              out_byte &= ~(0x80 >> col % 8); // black
            }
            if ((7 == col % 8) || (col == w - 1)) // write that last byte! (for w%8!=0 boarder)
            {
              output_row_color_buffer[out_idx] = out_color_byte;
              output_row_mono_buffer[out_idx++] = out_byte;
              out_byte = 0xFF; // white (for w%8!=0 boarder)
              out_color_byte = 0xFF; // white (for w%8!=0 boarder)
            }
          } // end pixel
          uint16_t yrow = y + (flip ? h - row - 1 : row);
          display->writeImage(output_row_mono_buffer, output_row_color_buffer, x, yrow, w, 1);
          
          //display.setTextColor(GxEPD_WHITE,GxEPD_BLACK);
          
        } // end line
        Serial.print("loaded in "); Serial.print(millis() - startTime); Serial.println(" ms");
        
        display->display();
      }
    }
  }
  file.close();
  if (!valid)
  {
    Serial.println("bitmap format not handled.");
  }
}

void futbolin_display::drawBitmapFromSpiffs_Buffered(const char *filename, int16_t x, int16_t y, bool with_color, bool partial_update, bool overwrite)
{
  fs::File file;
  bool valid = false; // valid format to be handled
  bool flip = true; // bitmap is stored bottom-to-top
  uint32_t startTime = millis();
  if ((x >= display->width()) || (y >= display->height())) return;
  Serial.println();
  Serial.print("Loading image '");
  Serial.print(filename);
  Serial.println('\'');
#if defined(ESP32)
  file = SPIFFS.open(String("/") + filename, "r");
#else
  file = SPIFFS.open(filename, "r");
#endif
  if (!file)
  {
    Serial.print("File not found");
    return;
  }
  // Parse BMP header
  if (read16(file) == 0x4D42) // BMP signature
  {
    uint32_t fileSize = read32(file);
    uint32_t creatorBytes = read32(file);
    uint32_t imageOffset = read32(file); // Start of image data
    uint32_t headerSize = read32(file);
    uint32_t width  = read32(file);
    uint32_t height = read32(file);
    uint16_t planes = read16(file);
    uint16_t depth = read16(file); // bits per pixel
    uint32_t format = read32(file);
    if ((planes == 1) && ((format == 0) || (format == 3))) // uncompressed is handled, 565 also
    {
      Serial.print("File size: "); Serial.println(fileSize);
      Serial.print("Image Offset: "); Serial.println(imageOffset);
      Serial.print("Header size: "); Serial.println(headerSize);
      Serial.print("Bit Depth: "); Serial.println(depth);
      Serial.print("Image size: ");
      Serial.print(width);
      Serial.print('x');
      Serial.println(height);
      // BMP rows are padded (if needed) to 4-byte boundary
      uint32_t rowSize = (width * depth / 8 + 3) & ~3;
      if (depth < 8) rowSize = ((width * depth + 8 - depth) / 8 + 3) & ~3;
      if (height < 0)
      {
        height = -height;
        flip = false;
      }
      uint16_t w = width;
      uint16_t h = height;
      if ((x + w - 1) >= display->width())  w = display->width()  - x;
      if ((y + h - 1) >= display->height()) h = display->height() - y;
      //if (w <= max_row_width) // handle with direct drawing
      {
        valid = true;
        uint8_t bitmask = 0xFF;
        uint8_t bitshift = 8 - depth;
        uint16_t red, green, blue;
        bool whitish, colored;
        if (depth == 1) with_color = false;
        if (depth <= 8)
        {
          if (depth < 8) bitmask >>= depth;
          file.seek(54); //palette is always @ 54
          for (uint16_t pn = 0; pn < (1 << depth); pn++)
          {
            blue  = file.read();
            green = file.read();
            red   = file.read();
            file.read();
            whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
            colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
            if (0 == pn % 8) mono_palette_buffer[pn / 8] = 0;
            mono_palette_buffer[pn / 8] |= whitish << pn % 8;
            if (0 == pn % 8) color_palette_buffer[pn / 8] = 0;
            color_palette_buffer[pn / 8] |= colored << pn % 8;
          }
        }
        if (partial_update) display->setPartialWindow(x, y, w, h);
        else display->setFullWindow();
        display->firstPage();
        do
        {
          if (!overwrite) display->fillScreen(GxEPD_WHITE);
          uint32_t rowPosition = flip ? imageOffset + (height - h) * rowSize : imageOffset;
          for (uint16_t row = 0; row < h; row++, rowPosition += rowSize) // for each line
          {
            uint32_t in_remain = rowSize;
            uint32_t in_idx = 0;
            uint32_t in_bytes = 0;
            uint8_t in_byte = 0; // for depth <= 8
            uint8_t in_bits = 0; // for depth <= 8
            uint16_t color = GxEPD_WHITE;
            file.seek(rowPosition);
            for (uint16_t col = 0; col < w; col++) // for each pixel
            {
              // Time to read more pixel data?
              if (in_idx >= in_bytes) // ok, exact match for 24bit also (size IS multiple of 3)
              {
                in_bytes = file.read(input_buffer, in_remain > sizeof(input_buffer) ? sizeof(input_buffer) : in_remain);
                in_remain -= in_bytes;
                in_idx = 0;
              }
              switch (depth)
              {
                case 24:
                  blue = input_buffer[in_idx++];
                  green = input_buffer[in_idx++];
                  red = input_buffer[in_idx++];
                  whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
                  colored = (red > 0x64) || ((green > 0x64) && (blue > 0x64)); // reddish or yellowish?
                  break;
                case 16:
                  {
                    uint8_t lsb = input_buffer[in_idx++];
                    uint8_t msb = input_buffer[in_idx++];
                    if (format == 0) // 555
                    {
                      blue  = (lsb & 0x1F) << 3;
                      green = ((msb & 0x03) << 6) | ((lsb & 0xE0) >> 2);
                      red   = (msb & 0x7C) << 1;
                    }
                    else // 565
                    {
                      blue  = (lsb & 0x1F) << 3;
                      green = ((msb & 0x07) << 5) | ((lsb & 0xE0) >> 3);
                      red   = (msb & 0xF8);
                    }
                    whitish = with_color ? ((red > 0x80) && (green > 0x80) && (blue > 0x80)) : ((red + green + blue) > 3 * 0x80); // whitish
                    colored = (red > 0xF0) || ((green > 0xF0) && (blue > 0xF0)); // reddish or yellowish?
                  }
                  break;
                case 1:
                case 4:
                case 8:
                  {
                    if (0 == in_bits)
                    {
                      in_byte = input_buffer[in_idx++];
                      in_bits = 8;
                    }
                    uint16_t pn = (in_byte >> bitshift) & bitmask;
                    whitish = mono_palette_buffer[pn / 8] & (0x1 << pn % 8);
                    colored = color_palette_buffer[pn / 8] & (0x1 << pn % 8);
                    in_byte <<= depth;
                    in_bits -= depth;
                  }
                  break;
              }
              if (whitish)
              {
                color = GxEPD_WHITE;
              }
              else if (colored && with_color)
              {
                color = GxEPD_COLORED;
              }
              else
              {
                color = GxEPD_BLACK;
              }
              uint16_t yrow = y + (flip ? h - row - 1 : row);
              display->drawPixel(x + col, yrow, color);
            } // end pixel
          } // end line
        }
        while (display->nextPage());
        Serial.print("loaded in "); Serial.print(millis() - startTime); Serial.println(" ms");
      }
    }
  }
  file.close();
  if (!valid)
  {
    Serial.println("bitmap format not handled.");
  }
}

uint16_t futbolin_display::read16(fs::File& f)
{
  // BMP data is stored little-endian, same as Arduino.
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t futbolin_display::read32(fs::File& f)
{
  // BMP data is stored little-endian, same as Arduino.
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}
