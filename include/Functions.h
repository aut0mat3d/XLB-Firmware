#ifndef Functions_h
#define Functions_h
#include <Globalvariables.h>

/******************** print some usage hints to serial **************************/

extern bool PrintHelp();


/******************** Logging to serial *****************************/

extern void printInt ( INT32U v, INT8U format );


// write CAN message details to serial port
extern void LogMsgToSerial ( bool Tx, XLBCANMsg* msg, INT8U format );

extern void ClearSerialInBuffer();

/******************** serial R/W CAN message *****************************/

// send CAN message to serial in binary mode
extern void SendMsgToSerial ( XLBCANMsg* msg );


// read CAN message from serial in binary mode
extern bool ReadMsgFromSerial ( XLBCANMsg* msg );


/******************** Serial Handling  **************************/
extern bool handleserial();


#endif