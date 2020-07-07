// CANUSB.INO
// Copyright (C) 2018 Thorsten Schmidt (tschmidt@ts-soft.de)
//     www.ts-soft.de

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

/**
 * Modifications by Rainer Oberegger:
 * 20200707: Choosed Resistors R220 for LEDs - Red steady on is way to bright - adopted the Code to have it flashing and a little dim
 *           Implemented a Define to not flood the Console with Can Bus Errors
 * 
 */ 
  

// these libraries are needed
#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>


#ifndef INT8U
#define INT8U byte
#endif

#ifndef INT16U
#define INT16U word
#endif

#ifndef INT32U
#define INT32U unsigned long
#endif

#ifndef INT64U
#define INT64U int64_t 
#endif

// NodeId and register for slave mode
#define ID_CONSOLE_MASTER           0x48
#define REG_CONSOLE_STATUS_SLAVE    0xD1

#define ID_BATTERY                  0x10
#define REG_BATTERY_CONFIG_SHUTDOWN 0x25

// Firmware version. Recently not used
// but may be helpful once for downward compatibility
// if protocol changes for some reason
#define XLB_Firmware_Version      100


// MCP_CAN init values
#define MCP_XTAL    MCP_8MHz
#define CAN_SPEED   CAN_125KBPS

// Serial init values
#define SER_SPEED   115200
#define SER_TIMEOUT 500

// used pins for MCP ChipSelect and Leds
#define MCP_CS_SPI  10
#define ERR_LED      8
#define RX_LED       6
#define TX_LED       4

// Led On/Off macros
#define RX_LED_ON       digitalWrite(RX_LED, HIGH)
#define RX_LED_OFF      digitalWrite(RX_LED, LOW)
#define TX_LED_ON       digitalWrite(TX_LED, HIGH)
#define TX_LED_OFF      digitalWrite(TX_LED, LOW)
#define ERROR_LED_ON    digitalWrite(ERR_LED, HIGH)
#define ERROR_LED_OFF   digitalWrite(ERR_LED, LOW)

// create CAN instance, set CS pin
MCP_CAN CAN(MCP_CS_SPI);

const
  char XLBPreamble[] = "CM";

struct XLBCANMsg
{
  INT32U Id;
  INT8U Len;
  INT8U Data[8];
};

bool errorprinted = false;

void setup()
{
  // configure LED pins as output
  pinMode(ERR_LED,OUTPUT);
  pinMode(RX_LED,OUTPUT);
  pinMode(TX_LED,OUTPUT);

  // switch off the LEDs
  ERROR_LED_OFF;
  RX_LED_OFF;
  TX_LED_OFF;
  
START_INIT:
  // init serial port
  Serial.begin(SER_SPEED);
  Serial.setTimeout(SER_TIMEOUT);
  
  // init can bus : baudrate, crystal
  if (CAN_OK == CAN.begin(CAN_SPEED, MCP_XTAL))                  
  {
    Serial.println(F("XLB Adapter ready!"));
  }
  else
  {
    //ERROR_LED_ON;
    if ((errorprinted == false) && (millis() > 2000))
    {
      Serial.println(F("Error: CAN init failed. Check connection Arduino<-->CAN adapter"));
      errorprinted = true;
    }
    
    for (uint8_t i = 0; i<20; i++)//Dirty flashing and dimming (LED with R220 is way to bright here) ;)
    {
      ERROR_LED_ON;
      delayMicroseconds(300);
      ERROR_LED_OFF;
      delayMicroseconds(200);
    }
    delay(90);
    
    goto START_INIT;
  }
  ERROR_LED_OFF;
}

/******************** some  tools *****************************/                                                                                                                                                                                                           

// init CANMsg structure
void InitCANMsg ( XLBCANMsg* msg, INT32U canId, INT8U len, INT16U Register, INT16U Value=0 )
{
  memset ( msg, 0, sizeof(XLBCANMsg));
  msg->Id = canId;
  msg->Len = len;
  msg->Data[0] = highByte(Register);
  msg->Data[1] = lowByte(Register);
  msg->Data[2] = highByte(Value);
  msg->Data[3] = lowByte(Value);
}

// waste any data in serial rx buffer
void ClearSerialInBuffer()
{
  while (Serial.available())
    Serial.read();  
}

/******************** Logging to serial *****************************/

void printInt ( INT32U v, INT8U format )
{
  Serial.print(' ');
  switch (format)
  {
    case DEC:
      if (v<100)
        Serial.print(0);
      if (v<10)
        Serial.print(0);
      break;
    case HEX:
      if (v<16)
        Serial.print(0);
      break;
  }    
  Serial.print(v,format);
}

// write CAN message details to serial port
void LogMsgToSerial ( bool Tx, XLBCANMsg* msg, INT8U format )
{
  if (Tx)
    Serial.print ( F("->") );
  else
    Serial.print ( F("<-") );
  printInt ( msg->Id, format );
  printInt ( msg->Len, format );
  for(INT8U i = 0; i<min(msg->Len,8); i++)
  {
    printInt ( msg->Data[i], format );
  }
  Serial.println ();
}

/******************** serial R/W CAN message *****************************/

// send CAN message to serial in binary mode
void SendMsgToSerial ( XLBCANMsg* msg )
{
  Serial.write(XLBPreamble);
  Serial.write((byte*)&msg->Id, sizeof(XLBCANMsg));
}


// read CAN message from serial in binary mode
bool ReadMsgFromSerial ( XLBCANMsg* msg )
{
  bool res = false;
  // skip, if there aren't enough bytes in the inbuffer
  // this prevents read a XLBCANMsg incomplete and run into timeout
  if (Serial.available()>=(long)sizeof(XLBCANMsg)+2)
  {
    // sync to the 'CM' prefix. Read and therefore consume all incoming
    // (nonsens?) bytes until 'CM' is found. The following 13 byte are
    // a CAN message and we have checked above, there are enough bytes
    // left in buffer for a complete message.
    if (Serial.read()==XLBPreamble[0])
    {
      if (Serial.read()==XLBPreamble[1])
      {
        memset ( msg, 0, sizeof(XLBCANMsg));
        res =Serial.readBytes((byte*)&msg->Id, sizeof(XLBCANMsg))==sizeof(XLBCANMsg);
      }
    }
  }
  return res;
}

/******************** CANbus R/W CAN message *****************************/

// receive a CAN message from CAN bus into message structure
bool ReadMsgFromCAN ( XLBCANMsg* msg )
{
  bool res;
  
  RX_LED_ON;
  ERROR_LED_OFF;
  memset ( msg, 0, sizeof(XLBCANMsg));
  res=(CAN.readMsgBufID(&msg->Id,&msg->Len, msg->Data)==CAN_OK);    // read data,  len: data length, buf: data buf
  if (!res)
    ERROR_LED_ON;
  RX_LED_OFF;
  return res;
}

// send CAN messase to CAN bus
bool SendMsgToCAN ( XLBCANMsg* msg )
{
  bool res;
  
  TX_LED_ON;
  ERROR_LED_OFF;
  res=(CAN.sendMsgBuf( msg->Id, 0, msg->Len, msg->Data)==CAN_OK);
  if (!res)
    ERROR_LED_ON;
  TX_LED_OFF;
  return res;
}

/******************** interactive CAN bus logging **************************/

// logs traffic on CAN bus to serial port
bool LoggingLoop()
{
  XLBCANMsg msg;
  INT8U format;

  if (toupper(Serial.read())=='D' )
    format = DEC;
  else
    format = HEX;

  ClearSerialInBuffer();

  Serial.println ( F("logging started") );
  while (1)
  {
    if (CAN.checkReceive()==CAN_MSGAVAIL)           // check if data coming
    {
      if (ReadMsgFromCAN ( &msg ))
        LogMsgToSerial ( false, &msg, format );
    }
    if (Serial.available()>0)
    {
      if (toupper(Serial.read()) == 'Q' )
      {
        Serial.println ( F("logging stopped") );
        break;
      }        
    }
  }
  return true;
}

/******************** interactive read command **************************/

// send a read message to CAN bus ( BionX specific )
bool ReadRegister ( INT32U canId, INT8U Register )
{
  XLBCANMsg msg;
  bool res = false;

  InitCANMsg ( &msg, canId, 2, Register );
  if (SendMsgToCAN ( &msg ))
  {
    LogMsgToSerial ( true, &msg, HEX );
    // todo: implement a wait loop with timeout
    delay(100);
    if (CAN.checkReceive()==CAN_MSGAVAIL)           // check if data coming
    {
      if (ReadMsgFromCAN ( &msg ))
      {
        LogMsgToSerial ( false, &msg, HEX );
        res = true;
      }
    }
  }
  return res;
}
// read Id and register from serial and execute read command
bool ReadCmd()
{
  INT32U canId;
  INT8U Register;
  
  canId = Serial.parseInt();
  if (canId!=0)
  {
    Register = Serial.parseInt();
    if (Register!=0)
    {
      return ReadRegister ( canId, Register );
    }
  }
  return false;
}

/******************** interactive erite command **************************/

// send a write message to CAN bus ( BionX specific )
bool WriteRegister ( INT32U canId, INT8U Register, INT8U Value )
{
  bool res = false;
  XLBCANMsg msg;
  InitCANMsg ( &msg, canId, 4, Register, Value );
  if (SendMsgToCAN ( &msg ))
  {
    LogMsgToSerial ( true, &msg, HEX );
    res = true;
  }
  return res;
}

// read Id, register and valuefrom serial and execute write command
bool WriteCmd()
{
  INT32U canId;
  INT8U Register;
  INT8U Value;

  canId = Serial.parseInt();
  if (canId!=0)
  {
    Register = Serial.parseInt();
    if (Register!=0)
    {
      Value = Serial.parseInt();
      // this is ugly: a missing 3rd param is interpreted as 0
      // instead of error. But how else could we set a 0 value?
//      if (Value!=0)
      {
        return WriteRegister ( canId, Register, Value );
      }
    }
  }
  return false;
}

/******************** transparent serial <--> CAN bus exchange **************************/

bool GatewayLoop()
{
  XLBCANMsg msg;
  Serial.println ( F("start gateway mode") );
  ClearSerialInBuffer();
  while(1)
  {
    // check if data coming
    if (CAN_MSGAVAIL == CAN.checkReceive())           
    {
      // read them from CAN and forward to serial
      if (ReadMsgFromCAN ( &msg ))
        SendMsgToSerial ( &msg );
    };

    // check for data in serial
    if (Serial.available())
    {
      // check for quit gateway mode
      if(toupper(Serial.peek())=='Q')
        break;

      if (ReadMsgFromSerial(&msg))
        SendMsgToCAN ( &msg );
    }
  }  
  Serial.println ( F("end gateway mode") );
  return true;
}

/******************** set to slave mode *****************************************/

bool SetToSlave()
{
  return WriteRegister ( ID_CONSOLE_MASTER, REG_CONSOLE_STATUS_SLAVE, 1 );
}

/******************** shutdown system *******************************************/

bool Shutdown()
{
  return WriteRegister ( ID_BATTERY, REG_BATTERY_CONFIG_SHUTDOWN, 1 );
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

/******************** some tests ************************************************/


bool Test()
{
  XLBCANMsg msg;
  InitCANMsg ( &msg, 0x10, 2, 0x74 );
  return SendMsgToCAN(&msg);
}

/* some tests with the MCP filters
bool SetFilter()
{
  CAN.init_Mask ( 0, 0, 0x7FFF00FF );
  CAN.init_Mask ( 1, 0, 0x7FFF00FF );
  CAN.init_Filt ( 0, 0, 0x005800D1 );
  CAN.init_Filt ( 1, 0, 0x00 );
  CAN.init_Filt ( 2, 0, 0x00 );
  CAN.init_Filt ( 3, 0, 0x00 );
  CAN.init_Filt ( 4, 0, 0x00 );
  CAN.init_Filt ( 5, 0, 0x00 );
  return true;
}

bool ClearFilter()
{
  CAN.init_Mask ( 0, 0, 0x0000 );
  CAN.init_Mask ( 1, 0, 0x0000 );
  CAN.init_Filt ( 0, 0, 0x0000 );
  return true;
}
*/

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
