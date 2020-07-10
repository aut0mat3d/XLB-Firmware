#ifndef globalvariables_h
#define globalvariables_h



extern const char XLBPreamble[];

struct XLBCANMsg
{
  INT32U Id;
  INT8U Len;
  INT8U Data[8];
};
extern XLBCANMsg Id; 
extern XLBCANMsg Len; 
extern XLBCANMsg Data[]; 

extern bool errorprinted;

extern uint8_t responsedata[];
//-------- Serial Stuff ---------
extern char input_line [];
extern unsigned int input_pos;


#endif