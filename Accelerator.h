
#ifndef __ACCELERATOR_H__
#define __ACCELERATOR_H__

#include "nvhls_pch.h"

#include <ac_reset_signal_is.h>

#include <axi/axi4.h>
#include "AxiSlaveToReg2.h"
#include <CombinationalBufferedPorts.h>

#define TAPS 16
#define TSTEP 32

class Accelerator : public sc_module {
 public:
  static const int kDebugLevel = 4;

  typedef axi::axi4<axi::cfg::standard> axi_;
  enum { numReg = 14, baseAddress = 0x0, numAddrBitsToInspect = 16 };

  sc_in<bool> clk;
  sc_in<bool> reset_bar;

  typename axi_::read::template slave<> axi_read;
  typename axi_::write::template slave<> axi_write;

  AxiSlaveToReg2<axi::cfg::standard, numReg, numAddrBitsToInspect> slave;
  typedef AxiSlaveToReg2<axi::cfg::standard, numReg, numAddrBitsToInspect>::reg_write reg_write_;

  sc_signal<NVUINTW(numAddrBitsToInspect)> baseAddr;
  sc_signal<NVUINTW(axi_::DATA_WIDTH)> regOut_chan[numReg];

  Connections::CombinationalBufferedPorts<reg_write_,0,1> regIn_chan;



  SC_HAS_PROCESS(Accelerator);

  Accelerator(sc_module_name name)
      : sc_module(name),
        clk("clk"),
        reset_bar("reset_bar"),
        axi_read("axi_read"),
        axi_write("axi_write"),
        slave("slave"),
        regIn_chan("regIn_chan")
  {
    slave.clk(clk);
    slave.reset_bar(reset_bar);

    slave.if_axi_rd(axi_read);
    slave.if_axi_wr(axi_write);
    slave.regIn(regIn_chan);

    slave.baseAddr(baseAddr);
    baseAddr.write(baseAddress);

    for (int i = 0; i < numReg; i++) {
      slave.regOut[i](regOut_chan[i]);
    }

    SC_THREAD (run); 
    sensitive << clk.pos(); 
    NVHLS_NEG_RESET_SIGNAL_IS(reset_bar);

  }

  void run()
  {
    // cout << sc_time_stamp() << " " << name() << " Starting" << endl;
    regIn_chan.ResetWrite();
    NVUINTW(axi_::DATA_WIDTH) lastCtrl = 0;
    NVUINTW(axi_::DATA_WIDTH) coeffReg = 0;
    NVUINTW(axi_::DATA_WIDTH-48) coeff[TAPS] = 0;
    NVUINTW(axi_::DATA_WIDTH) inputReg = 0;
    NVUINTW(axi_::DATA_WIDTH-48) input[TAPS] = 0;
    NVUINTW(axi_::DATA_WIDTH-48) result , result_old = 0;
    NVUINTW(axi_::DATA_WIDTH) temp = 0;
    NVUINTW(axi_::DATA_WIDTH) resultReg = 0;
    reg_write_ regwr;
    regwr.addr = 10*8;
    regwr.data = 0;
    // cout << sc_time_stamp() << " " << name() << " Reset" << endl;
    while (1)
    {
      wait();
      if (regOut_chan[1].read()!=lastCtrl) 
      {
        lastCtrl = regOut_chan[1].read();
        if (regOut_chan[1].read() == 2) {
          // Copy data from weight register to corresponding output register
          coeffReg=regOut_chan[2].read();
          //cout << "coeffReg " << coeffReg << endl;
          coeff[0] = (coeffReg >> (0 * 16)) & 0xFFFF;
          coeff[1] = (coeffReg >> (1 * 16)) & 0xFFFF;
          coeff[2] = (coeffReg >> (2 * 16)) & 0xFFFF;
          coeff[3] = (coeffReg >> (3 * 16)) & 0xFFFF;
          coeffReg=regOut_chan[3].read();
          coeff[4] = (coeffReg >> (0 * 16)) & 0xFFFF;
          coeff[5] = (coeffReg >> (1 * 16)) & 0xFFFF;
          coeff[6] = (coeffReg >> (2 * 16)) & 0xFFFF;
          coeff[7] = (coeffReg >> (3 * 16)) & 0xFFFF;
          coeffReg=regOut_chan[4].read();
          coeff[8] = (coeffReg >> (0 * 16)) & 0xFFFF;
          coeff[9] = (coeffReg >> (1 * 16)) & 0xFFFF;
          coeff[10] = (coeffReg >> (2 * 16)) & 0xFFFF;
          coeff[11] = (coeffReg >> (3 * 16)) & 0xFFFF;
          coeffReg=regOut_chan[5].read();
          coeff[12] = (coeffReg >> (0 * 16)) & 0xFFFF;
          coeff[13] = (coeffReg >> (1 * 16)) & 0xFFFF;
          coeff[14] = (coeffReg >> (2 * 16)) & 0xFFFF;
          coeff[15] = (coeffReg >> (3 * 16)) & 0xFFFF;
        }
        else if (regOut_chan[1].read() == 3 ) {
            inputReg = regOut_chan[6].read();
            for(int k = 0 ; k < 4 ; k++)
            {
              input[0] = (inputReg >> (k * 16)) & 0xFFFF;
              for (int i = 0; i < TAPS; i++)
              {
                  result = result_old + (input[i] * coeff[i]); // Compute the weighted sum.
                  result_old = result; // Update result_old for the next iteration.
                  temp = result;
              }
              for (int l = TAPS-1 ; l > 0; l--)
              {
                input[l] = input[l - 1]; // Shift the input values in the array.
              }
              resultReg = (resultReg) | (temp << (k*16));
              result = 0;
              result_old = 0; 
            }
            regwr.data= resultReg;
            regwr.addr= 80 ;
            // Start transfer
            cout << sc_time_stamp() << " " << name() << " Pushing " << regwr << endl;
            // Blocking Write
            regIn_chan.Push(regwr);
            regIn_chan.TransferNBWrite();
            cout << sc_time_stamp() << " " << name() << " Pushed" << endl;
            resultReg = 0; 
            inputReg = 0;    
            wait();
        } 
        else if (regOut_chan[1].read() == 4 ) {
            inputReg = regOut_chan[7].read();
            for(int k = 0 ; k < 4 ; k++)
            {
              input[0] = (inputReg >> (k * 16)) & 0xFFFF;
              for (int i = 0; i < TAPS; i++)
              {
                  result = result_old + (input[i] * coeff[i]); // Compute the weighted sum.
                  result_old = result; // Update result_old for the next iteration.
                  temp = result;
              }
              for (int l = TAPS-1 ; l > 0; l--)
              {
                input[l] = input[l - 1]; // Shift the input values in the array.
              }
              resultReg = (resultReg) | (temp << (k*16));
              result = 0;
              result_old = 0; 
            }
            regwr.data= resultReg;
            regwr.addr= 88 ;
            // Start transfer
            cout << sc_time_stamp() << " " << name() << " Pushing " << regwr << endl;
            // Blocking Write
            regIn_chan.Push(regwr);
            regIn_chan.TransferNBWrite();
            cout << sc_time_stamp() << " " << name() << " Pushed" << endl;
            resultReg = 0; 
            inputReg = 0;    
            wait();
        } 
        else if (regOut_chan[1].read() == 5 ) {
            inputReg = regOut_chan[8].read();
            //cout << "inputReg " << inputReg << endl;
            for(int k = 0 ; k < 4 ; k++)
            {
              input[0] = (inputReg >> (k * 16)) & 0xFFFF;
              for (int i = 0; i < TAPS; i++)
              {
                  result = result_old + (input[i] * coeff[i]); // Compute the weighted sum.
                  result_old = result; // Update result_old for the next iteration.
                  temp = result;
              }
              for (int l = TAPS-1 ; l > 0; l--)
              {
                input[l] = input[l - 1]; // Shift the input values in the array.
              }
              resultReg = (resultReg) | (temp << (k*16));
              result = 0;
              result_old = 0; 
            }
            regwr.data= resultReg;
            regwr.addr= 96 ;
            // Start transfer
            cout << sc_time_stamp() << " " << name() << " Pushing " << regwr << endl;
            // Blocking Write
            regIn_chan.Push(regwr);
            regIn_chan.TransferNBWrite();
            cout << sc_time_stamp() << " " << name() << " Pushed" << endl;
            resultReg = 0; 
            inputReg = 0;  
            wait();   
        }
        else if (regOut_chan[1].read() == 6 ) {
            inputReg = regOut_chan[9].read();
            for(int k = 0 ; k < 4 ; k++)
            {
              input[0] = (inputReg >> (k * 16)) & 0xFFFF;
              for (int i = 0; i < TAPS; i++)
              {
                result = result_old + (input[i] * coeff[i]); // Compute the weighted sum.
                result_old = result; // Update result_old for the next iteration.
                temp = result;
              }
              for (int l = TAPS-1 ; l > 0; l--)
              {
                input[l] = input[l - 1]; // Shift the input values in the array.
              }
              resultReg = (resultReg) | (temp << (k*16));
              result = 0;
              result_old = 0; 
            }
            regwr.data= resultReg;
            regwr.addr= 104 ;
            // Start transfer
            cout << sc_time_stamp() << " " << name() << " Pushing " << regwr << endl;
            // Blocking Write
            regIn_chan.Push(regwr);
            regIn_chan.TransferNBWrite();
            cout << sc_time_stamp() << " " << name() << " Pushed" << endl;
            resultReg = 0; 
            inputReg = 0;    
            wait(); 
        }
      }  
    }
  }  
};

#endif
