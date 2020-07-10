#include <Arduino.h>
#include <Defines.h>
#include <Globalvariables.h>
#include <Functions.h>
#include <CAN-Registers.h>
#include <CAN_Functions.h>


// create CAN instance, set CS pin
MCP_CAN CAN(MCP_CS_SPI);

/******************** some  tools *****************************/                                                                                                                                                                                                           

// init CANMsg structure
//void InitCANMsg ( XLBCANMsg* msg, INT32U canId, INT8U len, INT16U Register, INT16U Value=0 )
void InitCANMsg ( XLBCANMsg* msg, INT32U canId, INT8U len, INT16U Register, INT16U Value)
//Value = 0
{
  memset ( msg, 0, sizeof(XLBCANMsg));
  msg->Id = canId;
  msg->Len = len;
  msg->Data[0] = highByte(Register);
  msg->Data[1] = lowByte(Register);
  msg->Data[2] = highByte(Value);
  msg->Data[3] = lowByte(Value);
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

void clearresponsedata()
{
  for (uint8_t i = 0; i<8; i++)
  {
    responsedata[i] = 0x00;
  }
}

/******************** interactive read command **************************/
bool GetRegister ( INT32U canId, INT8U Register )
{
  clearresponsedata();
  XLBCANMsg msg;
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
        //LogMsgToSerial ( false, &msg, HEX );
        /**
         *struct XLBCANMsg
        {
          INT32U Id;
          INT8U Len;
          INT8U Data[8];
        };
         */
        DBGprintln("Parsed:");
        DBGprint("Id:");
        DBGprintln(msg.Id,HEX);
        DBGprint("Len:");
        DBGprintln(msg.Len);
        DBGprint("Data:");
        for(uint8_t i = 0; i < msg.Len; i++)
        {
          if (msg.Data[i] <10)
          {
            DBGprint("0");
          }
          DBGprint( msg.Data[i],HEX);
          DBGprint(" ");
        }
        DBGprintln();

        if (msg.Id == ID_CONSOLE_RESPONSE)//0x58 is ID BIB - adresses BIB 
        {
          DBGprintln("Response is ours");
          for(uint8_t i = 0; i < msg.Len; i++)
          {
            responsedata[i] = msg.Data[i];//I know this can be done more convinient - but hey - i am a beginner
          }
          return true;

        }

        
/*
         printInt ( msg->Id, format );
          printInt ( msg->Len, format );
          for(INT8U i = 0; i<min(msg->Len,8); i++)
          {
            printInt ( msg->Data[i], format );
          }
          */
         
        //LogMsgToSerial ( false, &msg, HEX );
        //res = true;
      }
    }
  }
  return false;

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
      /*
      DBGprint("canId:");
      DBGprint(canId);
      DBGprint(", Register:");
      DBGprint(Register);
      */
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
  uint8_t errcnt = 0;
  while (errcnt < 10)
  {
    DBGprint("Try:");
    DBGprintln(errcnt);
    WriteRegister ( ID_CONSOLE_SLAVE, REG_CONSOLE_STATUS_SLAVE, 1 );
    //delay(100);
    bool val = GetRegister ( ID_CONSOLE_SLAVE, REG_CONSOLE_STATUS_SLAVE );
    if (val == true)//Received a Packet to BIB (us)
    {
      /* Thats what i got here:
      Id:58
      Len:4
      Data:00 D1 00 01
      */
      if ((responsedata[3] == 0x01)) //am i doing right? this should indicate slave mode - who knows?
      {
        return true;
      }
    }
    errcnt++;
  }
  return false;
}

/******************** shutdown system *******************************************/

bool Shutdown()
{
  return WriteRegister ( ID_BATTERY, REG_BATTERY_CONFIG_SHUTDOWN, 1 );
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