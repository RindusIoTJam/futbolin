// Deshabilitando interrupciones de pin y antirrebote de 50ms
#include "RTClock.h"
#include ".\stm32sleep-master\src\STM32Sleep.cpp"
#include <SPI.h>
#include <LoRa_STM32.h>
#define right_goal PA10
#define left_goal PB8
#define reset_button PA0
RTClock rt(RTCSEL_LSE);
void right_goal_interrupt() ;
void left_goal_interrupt();
void reset_button_interrupt();
const char left = 'l';
const char right = 'r';
const char clear = 'c';
const char undo = 'n';
const unsigned long SECUREPRESSTIME = 20;
const unsigned long LONGPRESSTIME = 800;
volatile bool longpressure = false;
volatile char packet = 'c';
volatile int reset_counter = 0;
volatile bool flagReadyForExternalInterrupt = true;
enum state_machine {INTERRUPT, SEND_PACKET, RESET_PRESS, GO_TO_SLEEP};
volatile state_machine state = GO_TO_SLEEP;
// the setup function runs once when you press reset or power the board

void setup() 
{
  pinMode(right_goal,INPUT_PULLUP);
  pinMode(left_goal,INPUT_PULLUP);
  pinMode(reset_button,INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(right_goal),right_goal_interrupt,FALLING);
  attachInterrupt(digitalPinToInterrupt(left_goal),left_goal_interrupt,FALLING);
  attachInterrupt(digitalPinToInterrupt(reset_button),reset_button_interrupt,FALLING);
  //LoRa Module Initialization
  if (!LoRa.begin(433E6)) 
  { //initialize LoRa 433Mhz
    while (1);
  }  
  LoRa.end();
}
// the loop function runs over and over again forever
void loop()
{
  switch (state)
  {
  case INTERRUPT:
    switchToPLLwithHSE(RCC_PLLMUL_9); //recover normal clock speed
    if (checkInterrupt(packet))
    {
      state = SEND_PACKET;
    }else
    {
      state = GO_TO_SLEEP;
    }
  break;
  case SEND_PACKET:
    while (!LoRa.begin(433E6));  //transmit lora error using led
    LoRa.beginPacket();  
    switch (packet)
    {
    case 'l':
      LoRa.print(left);
    break;
        case 'r':
      LoRa.print(right);
    break;
        case 'c':
      if(longpressure)
      {
        longpressure = false;
        LoRa.print(undo);
      }
      else
      {
        LoRa.print(clear);
      }
    break;
      }
      LoRa.endPacket();
      LoRa.end();
      state = GO_TO_SLEEP;
    break;
    case GO_TO_SLEEP:
      //attachInterrupt(digitalPinToInterrupt(right_goal),right_goal_interrupt,FALLING);
      //attachInterrupt(digitalPinToInterrupt(left_goal),left_goal_interrupt,FALLING);
      //attachInterrupt(digitalPinToInterrupt(reset_button),reset_button_interrupt,FALLING);
      flagReadyForExternalInterrupt = true; 
      goToSleep();
      break;
    default:
      break;   
  }   
}    
void right_goal_interrupt() 
{
  if (flagReadyForExternalInterrupt)
  {
    flagReadyForExternalInterrupt = false;  
    state = INTERRUPT;    //state r right goal
    packet = 'r';
  }
}
void left_goal_interrupt()
{
  if (flagReadyForExternalInterrupt)
  {
    flagReadyForExternalInterrupt = false;  
    state = INTERRUPT;    //state l left goal
    packet = 'l';
  }
}
void reset_button_interrupt()
{
  if (flagReadyForExternalInterrupt)
  {
    flagReadyForExternalInterrupt = false;  
    state = INTERRUPT;    //state c clear
    packet = 'c';
  }
}
void goToSleep()
{
  goToSleep(STOP); //wait for a goal  
}
bool checkInterrupt(char packet)
{
  unsigned long previousmillis = millis();
  unsigned long nowmillis = 0;
  switch (packet)
  {
    case 'l':
      while(digitalRead(left_goal) == 0);
    break;
    case 'r':
      while(digitalRead(right_goal) == 0);
    break;
    case 'c':
      while(digitalRead(reset_button) == 0);
    break;
  }
  nowmillis = millis();
  
  if(nowmillis - previousmillis > LONGPRESSTIME)
  {
    longpressure = true;
  }
  
  return (nowmillis - previousmillis > SECUREPRESSTIME);
}
