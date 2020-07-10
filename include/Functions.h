#ifndef Functions_h
#define Functions_h
#include <Globalvariables.h>
/******************** Logging to serial *****************************/

extern void printInt ( INT32U v, INT8U format );


// write CAN message details to serial port
extern void LogMsgToSerial ( bool Tx, XLBCANMsg* msg, INT8U format );


/******************** serial R/W CAN message *****************************/

// send CAN message to serial in binary mode
extern void SendMsgToSerial ( XLBCANMsg* msg );


// read CAN message from serial in binary mode
extern bool ReadMsgFromSerial ( XLBCANMsg* msg );



#endif