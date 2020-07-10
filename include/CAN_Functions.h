#ifndef CANFunctions_h
#define CANFunctions_h

#include <mcp_can.h>
#include <mcp_can_dfs.h>

#ifndef CANINSTANCEDONE
// create CAN instance, set CS pin
MCP_CAN CAN(MCP_CS_SPI);
#define CANINSTANCEDONE
#endif

/******************** some  tools *****************************/                                                                                                                                                                                                           

// init CANMsg structure
extern void InitCANMsg ( XLBCANMsg* msg, INT32U canId, INT8U len, INT16U Register, INT16U Value=0 );


//******************** CANbus R/W CAN message *****************************/

// receive a CAN message from CAN bus into message structure
extern bool ReadMsgFromCAN ( XLBCANMsg* msg );


// send CAN messase to CAN bus
extern bool SendMsgToCAN ( XLBCANMsg* msg );


/******************** interactive CAN bus logging **************************/

// logs traffic on CAN bus to serial port
extern bool LoggingLoop();


/******************** interactive read command **************************/

// send a read message to CAN bus ( BionX specific )
extern bool ReadRegister ( INT32U canId, INT8U Register );


// read Id and register from serial and execute read command
extern bool ReadCmd();


/******************** interactive erite command **************************/

// send a write message to CAN bus ( BionX specific )
extern bool WriteRegister ( INT32U canId, INT8U Register, INT8U Value );


// read Id, register and valuefrom serial and execute write command
extern bool WriteCmd();


/******************** transparent serial <--> CAN bus exchange **************************/

extern bool GatewayLoop();


/******************** set to slave mode *****************************************/

extern bool SetToSlave();


/******************** shutdown system *******************************************/

extern bool Shutdown();

/******************** some tests ************************************************/


extern bool Test();


/* some tests with the MCP filters
extern bool SetFilter();


extern bool ClearFilter();

*/



#endif