
// these libraries are needed
#include <Arduino.h>
#include <SPI.h>


#include <Defines.h>
#include <Globalvariables.h>
#include <Functions.h>
#include <CAN_Functions.h>


void setup()
{
  #ifdef USELEDS
  // configure LED pins as output
  pinMode(ERR_LED,OUTPUT);
  pinMode(RX_LED,OUTPUT);
  pinMode(TX_LED,OUTPUT);
  #endif

  // switch on LEDs (Functional Test)
  ERROR_LED_ON;
  delay(100);
  RX_LED_ON;
  delay(100);
  TX_LED_ON;
  delay(100);

  // switch off the LEDs
  ERROR_LED_OFF;
  delay(100);
  RX_LED_OFF;
  delay(100);
  TX_LED_OFF;
  delay(100);
  
START_INIT:
  // init serial port
  Serial.begin(SER_SPEED);
  Serial.setTimeout(SER_TIMEOUT);
  
  // init can bus : baudrate, crystal
  if (CAN_OK == CAN.begin(CAN_SPEED, MCP_XTAL))                  
  {
    Serial.println(F("XLB Adapter ready!"));
    //Serial.println(F("Enter ? for Commands"));
  }
  else
  {
    //Print Error Message (only once)
    if ((errorprinted == false) && (millis() > 1000))//1 Second to have Terminal on PC up
    {
      Serial.println(F("Error: CAN init failed. Check connection Arduino<-->CAN adapter"));
      errorprinted = true;
    }
    
    //Serial.println(F("Error: CAN init failed. Check connection Arduino<-->CAN adapter"));
    
    for (uint8_t i = 0; i<19; i++)//Dirty flashing and dimming (LED with R510 is way to bright here) ;)
    {
      ERROR_LED_ON;
      delayMicroseconds(500);
      ERROR_LED_OFF;
      delayMicroseconds(100);
    }
    delay(80);
    
    goto START_INIT;
  }
  ERROR_LED_OFF;
}


/******************** main program loop **************************/

void loop()
{
  //bool res;
  
  if (Serial.available()>0)
  {
    bool res = handleserial();

    if (!res)
      Serial.println(F("error"));
    ClearSerialInBuffer();
  }
  else
    delay(100);
}


/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
