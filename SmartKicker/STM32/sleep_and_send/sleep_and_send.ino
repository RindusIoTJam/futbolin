#include "RTClock.h"
#include ".\stm32sleep-master\src\STM32Sleep.cpp"
#include <SPI.h>
#include <LoRa_STM32.h>

#define right_goal PB1
#define left_goal PB0


//#define LORA_NOT_AVAILABLE 1
//#define DEBUG_LED 1

volatile char goal_source = 'c';

void right_goal_interrupt() 
{
  goal_source = 'r';
}
void left_goal_interrupt()
{
  goal_source = 'l';
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
#ifdef DEBUG_LED
  pinMode(LED_BUILTIN, OUTPUT); //Testing
  digitalWrite(LED_BUILTIN, HIGH); //led is inverted in my board
#endif

  pinMode(right_goal,INPUT_PULLUP);
  pinMode(left_goal,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(right_goal),right_goal_interrupt,FALLING);
  attachInterrupt(digitalPinToInterrupt(left_goal),left_goal_interrupt,FALLING);
  
#ifndef LORA_NOT_AVAILABLE
  if (!LoRa.begin(433E6)) { //transmit lora error using led
    while (1)
    {
      #ifdef DEBUG_LED
      digitalWrite(LED_BUILTIN, LOW);   
      delay(100);                       
      digitalWrite(LED_BUILTIN, HIGH);    
      delay(100); 
      #endif
    }
  }
#endif
  #ifndef LORA_NOT_AVAILABLE    
  LoRa.beginPacket();
  LoRa.print(goal_source);
  LoRa.endPacket();
  #endif
  goToSleep(STOP); //wait for a goal

}

// the loop function runs over and over again forever
void loop() {

  switchToPLLwithHSE(RCC_PLLMUL_9); //recover normal clock speed

#ifdef DEBUG_LED
  if(goal_source == 'r')
  {
    digitalWrite(LED_BUILTIN, LOW);   //only for debug 
    delay(100);                       
    digitalWrite(LED_BUILTIN, HIGH);   
    delay(100);          
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);   //only for debug 
    delay(100);                       
    digitalWrite(LED_BUILTIN, HIGH);   
    delay(100); 
    digitalWrite(LED_BUILTIN, LOW);   //only for debug 
    delay(100);                       
    digitalWrite(LED_BUILTIN, HIGH);   
    delay(100); 
  }
#endif 

#ifndef LORA_NOT_AVAILABLE    
  LoRa.beginPacket();
  LoRa.print(goal_source);
  LoRa.endPacket();
#endif
  goToSleep(STOP); //goal sent , wait for a new one
  
}
