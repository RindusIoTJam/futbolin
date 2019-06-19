#include "RTClock.h"
#include ".\stm32sleep-master\src\STM32Sleep.cpp"
#include <SPI.h>
#include <LoRa_STM32.h>

#define right_goal PB1
#define left_goal PB0
#define reset_button PB11

#define Right 'r'
#define Left 'l'
#define Null 'n'
#define Clear 'c' 

volatile char goal_source = '\0';
volatile int reset_counter = 0;

//volatile int reset_noise_filter=0;
//volatile int right_noise_filter=0;
//volatile int left_noise_filter=0;

void right_goal_interrupt() 
{
 
  goal_source = Right;
  
}
void left_goal_interrupt()
{
  
  goal_source = Left;
 
}
void reset_button_interrupt()
{
  int reset_noise_filter=0;
  for(int i = 0; i < 40; i++)
  {
    reset_noise_filter += (int)digitalRead(reset_button);
    delay(11);
  }
  if(reset_noise_filter > 30)
  { 
     goal_source = Null;
  }
  else if (reset_noise_filter > 8)
  {
    goal_source = Clear;
  }
  reset_noise_filter = 0;
  
}

// the setup function runs once when you press reset or power the board
void setup() {

  pinMode(right_goal,INPUT_PULLUP);
  pinMode(left_goal,INPUT_PULLUP);
  pinMode(reset_button,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(right_goal),right_goal_interrupt,FALLING);
  attachInterrupt(digitalPinToInterrupt(left_goal),left_goal_interrupt,FALLING);
  attachInterrupt(digitalPinToInterrupt(reset_button),reset_button_interrupt,FALLING);


  if (!LoRa.begin(433E6)) {
    while (1){}
  }  
  
  LoRa.beginPacket();
  LoRa.print(Clear);
  LoRa.endPacket();

  //LoRa.sleep();
  goToSleep(STOP); //wait for a goal

}

// the loop function runs over and over again forever
void loop() {

  switchToPLLwithHSE(RCC_PLLMUL_9); //recover normal clock speed
  LoRa.idle();
    switch(goal_source)
    {
      case Right:
      case Left:
      case Null:
      case Clear: 
    LoRa.beginPacket();
    LoRa.print(goal_source);
    LoRa.endPacket();
    break;
    default:
    break;
    }
  
  goal_source = '\0';
  //LoRa.sleep();
  goToSleep(STOP); //goal sent , wait for a new one
  
}
