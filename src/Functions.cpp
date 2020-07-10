#include <Arduino.h>
#include <Defines.h>
#include <Globalvariables.h>
#include <CAN-Registers.h>
#include <Functions.h>


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
