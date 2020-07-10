#ifndef globalvariables_h
#define globalvariables_h

extern const char XLBPreamble[];


struct XLBCANMsg
{
  INT32U Id;
  INT8U Len;
  INT8U Data;
};
XLBCANMsg obj = {3}; 

extern bool errorprinted;

#endif