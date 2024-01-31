
#include <stdio.h>

#define TAPS 16
#define TSTEP 48

// short coef[TAPS] = {
// #include "coef.inc"
// };

// short input[TSTEP] = {
// #include "input.inc"
// };


#include "expected.inc"

int main( int argc, char* argv[] )
{
  volatile long long *llp;
  volatile long long **llpp;

  llpp=(volatile long long**)0x70000010; // dma sr
  *llpp=(volatile long long*)0x00004000; // memctl coef1 address
  llpp=(volatile long long**)0x70000018; // dma dr
  *llpp=(volatile long long*)0x10010010; // fir tap coef
  llp=(volatile long long*)0x70000020;   // dma len
  *llp=(volatile long long)32; // starts transfer
  llp=(volatile long long*)0x70000000;   // dma st

  // The following line polls the dma st register until it is 0,
  // effectively waiting until the transfer is complete.
  while (*llp); 

  // If the previous line is omitted, then the next one will not
  // show the expected valye, because the transfer is not complete.
  printf("cpu main {W[3],W[2],W[1],W[0]} 0x%lx (0x2ffffffff0000 expected)\n",*((long long*)0x70010010));

  llp=(volatile long long*)0x70010008; // fir ctrl
  *llp=(volatile long long)0x02;       // get coefficients 

  

  llpp=(volatile long long**)0x70000010; // dma sr
  *llpp=(volatile long long*)0x00002000; // memctl input address
  llpp=(volatile long long**)0x70000018; // dma dr
  *llpp=(volatile long long*)0x10010030; // fir input address
  llp=(volatile long long*)0x70000020;   // dma len
  *llp=(volatile long long)32; // starts transfer
  llp=(volatile long long*)0x70000000;   // dma st

  while (*llp); 

  printf("cpu main {I[3],I[2],I[1],I[0]} 0x%lx ( 0x200240045003e expected)\n",*((long long*)0x70010030));

  llp=(volatile long long*)0x70010008; // fir ctrl
  *llp=(volatile long long)0x03;       // get first set of input reg and start computation

  printf("1ST Input Reg Computation start for 1ST set \n");

  llp=(volatile long long*)0x70010008; // fir ctrl
  *llp=(volatile long long)0x04;       // get second set of input reg and start computation

  printf("2ND Input Reg Computation start for 1ST set\n");

  llp=(volatile long long*)0x70010008; // fir ctrl
  *llp=(volatile long long)0x05;       // get third set of input reg and start computation

  printf("3RD Input Reg Computation start for 1ST set\n");

  llp=(volatile long long*)0x70010008; // fir ctrl
  *llp=(volatile long long)0x06;       // get fourth set of input reg and start computation

  printf("4TH Input Reg Computation start for 1ST set\n");


  // Writing first set of outputs to memory
  llpp=(volatile long long**)0x70000010; // dma sr
  *llpp=(volatile long long*)0x10010050; // fir output address
  llpp=(volatile long long**)0x70000018; // dma dr
  *llpp=(volatile long long*)0x00001000; // memctl output address
  llp=(volatile long long*)0x70000020;   // dma len
  *llp=(volatile long long)32; // starts transfer
  llp=(volatile long long*)0x70000000;   // dma st

  while (*llp); 

  printf("cpu main {O[3],O[2],O[1],O[0]} 0x%lx (0x13FF7DFFC22 0000 expected)\n",*((long long*)0x70010050));

  llpp=(volatile long long**)0x70000010; // dma sr
  *llpp=(volatile long long*)0x00002020; // memctl input address
  llpp=(volatile long long**)0x70000018; // dma dr
  *llpp=(volatile long long*)0x10010030; // fir input address
  llp=(volatile long long*)0x70000020;   // dma len
  *llp=(volatile long long)32; // starts transfer
  llp=(volatile long long*)0x70000000;   // dma st

  while (*llp); 

  printf("cpu main {I[3],I[2],I[1],I[0]} 0x%lx (0xFFE9FFC3FFBAFFE5 expected)\n",*((long long*)0x70010030));



  llp=(volatile long long*)0x70010008; // fir ctrl
  *llp=(volatile long long)0x03;       // get first set of input reg and start computation

  printf("1ST Input Reg Computation start for 2ND set\n");

  llp=(volatile long long*)0x70010008; // fir ctrl
  *llp=(volatile long long)0x04;       // get second set of input reg and start computation

  printf("2ND Input Reg Computation start for 2ND set\n");

  llp=(volatile long long*)0x70010008; // fir ctrl
  *llp=(volatile long long)0x05;       // get third set of input reg and start computation

  printf("3RD Input Reg Computation start for 2ND set\n");

  llp=(volatile long long*)0x70010008; // fir ctrl
  *llp=(volatile long long)0x06;       // get fourth set of input reg and start computation

  printf("4TH Input Reg Computation start for 2ND set\n");


  // Writing second set of outputs to memory
  llpp=(volatile long long**)0x70000010; // dma sr
  *llpp=(volatile long long*)0x10010050; // fir output address
  llpp=(volatile long long**)0x70000018; // dma dr
  *llpp=(volatile long long*)0x00001020; // memctl output address
  llp=(volatile long long*)0x70000020;   // dma len
  *llp=(volatile long long)32; // starts transfer
  llp=(volatile long long*)0x70000000;   // dma st

  while (*llp); 

  printf("cpu main {O[3],O[2],O[1],O[0]} 0x%lx (0x658EB58DB24DAC7 expected)\n",*((long long*)0x70010050));
  
  short error,total_error;
  short *output=(short *)0x60001000;
  total_error=0;
  for (int n=0; n<TSTEP-TAPS; n++) {
    error=expected[n]-output[n];              // Error for this time-step
    total_error+=(error<0)?(-error):(error);  // Absolute value
    // Uncomment the next line for a detailed error check
    //printf("cpu main k: %d output: %d expected %d\n",n,output[n],expected[n]);
  }

  printf("cpu main error: %d\n",total_error);

  llp=(volatile long long*)0x70010008;; // fir ctrl
  //*llp=(volatile long long)0x01;        // Print registers
  *llp=(volatile long long)0x0f;        // Exit

  return 0;
}
