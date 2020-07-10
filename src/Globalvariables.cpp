#include <Arduino.h>
#include <Defines.h>
#include <Globalvariables.h>


const char XLBPreamble[] = "CM";

bool errorprinted = false;

uint8_t responsedata[8] = {0,0,0,0,0,0,0,0};
//-------- Serial Stuff ---------
char input_line [MAX_INPUT];
unsigned int input_pos = 0;
