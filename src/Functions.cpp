#include <Arduino.h>
#include <Defines.h>
#include <Globalvariables.h>
#include <CAN-Registers.h>
#include <Functions.h>
#include <CAN_Functions.h>


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

// waste any data in serial rx buffer
void ClearSerialInBuffer()
{
  while (Serial.available())
    Serial.read();  
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


bool processIncomingByte (byte inByte)
{
  bool res = false;
  //DBGprintln(inByte,HEX);
  switch (inByte)
    {
      case '?' :
      res =PrintHelp();
      break;

      case 'V' :
      if (input_pos == 0)
      {      
        res = ( Serial.println( XLB_Firmware_Version) > 0 );     
      }
      break;

      case 'G': //Gateway Mode
      if (input_pos == 0)
      {
        res = GatewayLoop();
      }
      break;

      case 'L' :
      res = LoggingLoop();
      break;

      case 'O' :
      res = Shutdown();
      break;

    case 'S' :
      res = SetToSlave();
      break;

      case 0x0D:   // '\r' carriage return
      case '\n':   // end of text
      input_line [input_pos] = 0;  // terminating null byte
      
      // terminator reached! process input_line here ...
      //process_data (input_line,input_pos);
      
      // reset buffer for next time
      input_pos = 0;  
      break;

      default:
      // keep adding if not full ... allow for terminating null byte
      if (input_pos < (MAX_INPUT - 1))
        input_line [input_pos++] = inByte;
      break;


    }
  
  return res;
}


/******************** Serial Handling  **************************/
bool handleserial()
{
  bool res = false;

  
  while (Serial.available())
  {
    res = processIncomingByte (Serial.read ());
  }
  return true;
  
  
  unsigned char inchar = Serial.read();
  switch (toupper(inchar))
  {
    break;
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
  return res;
}