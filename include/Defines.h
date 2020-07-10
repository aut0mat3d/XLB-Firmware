#ifndef defines_h
#define defines_h
#include <CAN-Registers.h>

// Firmware version. Recently not used
// but may be helpful once for downward compatibility
// if protocol changes for some reason
#define XLB_Firmware_Version      100

#define USELEDS //Uncomment if you want to use LEDs 
#define DEBUG


// MCP_CAN init values
#define MCP_XTAL    MCP_16MHz
#define CAN_SPEED   CAN_125KBPS

// Serial init values
#define SER_SPEED   115200
#define SER_TIMEOUT 500
#define MAX_INPUT 12    //Serial Buffer

#ifdef DEBUG
#define DBGprint(...) Serial.print(__VA_ARGS__)
#define DBGprintln(...) Serial.println(__VA_ARGS__)
#else
#define DBGprint(...)//Dummy when not in Debug - so we can have defines forgotten without a Hassle
#define DBGprintln(...)
#endif

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

//---------------------       Hardware Access      --------------------
#define MCP_CS_SPI  10

#ifdef USELEDS
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
#else
#define RX_LED_ON       __asm__ __volatile__("nop")
#define RX_LED_OFF      __asm__ __volatile__("nop")
#define TX_LED_ON       __asm__ __volatile__("nop")
#define TX_LED_OFF      __asm__ __volatile__("nop")
#define ERROR_LED_ON    __asm__ __volatile__("nop")
#define ERROR_LED_OFF   __asm__ __volatile__("nop")
#endif



#endif