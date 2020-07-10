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



#endif