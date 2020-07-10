
// these libraries are needed
#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>

#include <Defines.h>
#include <Functions.h>



// create CAN instance, set CS pin
MCP_CAN CAN(MCP_CS_SPI);

#include <Globalvariables.h>



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



/******************** print some usage hints to serial **************************/

bool PrintHelp()
{
  Serial.println ( F("") );
  Serial.println ( F("G                enter gateway mode. Transparent exchange messages between serial<-->CAN bus. Leave with Q") );
  Serial.println ( F("L{D}             enter logging mode. With LD log values decimal, default is hex. Leave with Q") );
  Serial.println ( F("R Id Reg         read register 'Reg' from CAN node 'Id'. Use decimal numbers.") );
  Serial.println ( F("W Id Reg Value   write 'Value' into register 'Reg' on CAN node 'Id'. Use decimal numbers") );
  Serial.println ( F("S                set bike to slave mode") );
  Serial.println ( F("V                show firmware version") );
  Serial.println ( F("O                shutdown system") );
  Serial.println ( F("") );
  return true;
}



/******************** main program loop **************************/

void loop()
{
  unsigned char inchar;
  bool res;
  
  if (Serial.available()>0)
  {
    inchar = Serial.read();
    switch (toupper(inchar))
    {
      case 'V' :
        res = ( Serial.println( XLB_Firmware_Version) > 0 );
        break;
      
      case 'G' :
        res = GatewayLoop();
        break;
        
      case 'L' :
        res = LoggingLoop();
        break;

      case 'R' :
        res = ReadCmd();
        break;
        
      case 'W' :
        res = WriteCmd();
        break;

      case '?' :
        res =PrintHelp();
        break;

      case 'O' :
        res = Shutdown();
        break;

      case 'S' :
        res = SetToSlave();
        break;
/*        
      case 'T' :
        res = Test();
        break;
*/
      default :
        res = false;
    }
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
