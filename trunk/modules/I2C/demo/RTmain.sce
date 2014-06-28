// 
// 
// This a template for writing real-time applications using OpenRTDynamics
// (openrtdynamics.sf.net)
// 
//

// 
// 
// A demonstration for using the I2C-interface.
// This demo was tested with a TLC59116 (16 channel pwm LED driver)
// device connected to adress 110 = 0x6E = 0xDC/2.
// The kernels I2C adresses seem to be the halt of the actual ones?
// 
// A complete driver for the TLC59116 is appended to the end of this
// file.
// 


// The name of the program
ProgramName = 'RTmain'; // must be the filename without .sce
thispath = get_absolute_file_path(ProgramName+'.sce');
cd(thispath);



//
// To run the generated controller stored in template.[i,r]par, call from a terminal the 
//
// ortd --baserate=1000 --rtmode 1 -s template -i 901 -l 0
// 
// If you want to use harder real-time capabilities, run as root: 
// 
// sudo ortd --baserate=1000 --rtmode 1 -s template -i 901 -l 0
// 



// The main real-time thread
function [sim, outlist, userdata] = Thread_MainRT(sim, inlist, userdata)
  // This will run in a thread
  [sim, Tpause] = ld_const(sim, ev, 1/20);  // The sampling time that is constant at 27 Hz in this example
  [sim, out] = ld_ClockSync(sim, ev, in=Tpause); // synchronise this simulation

  //
  // Add you own control system here
  //


  ev = 0;

  // generate a signal
  [sim, L1] = ld_play_simple(sim, ev, r=linspace(0,255,200) );
  [sim,L1_] = ld_floorInt32(sim, 0, in=L1); 

  // Open up an I2C device that is refered by "LEDArray1"
  [sim] = ld_I2CDevice_shObj(sim, 0, ObjectIdentifyer="LEDArray1", Visibility=2, I2CDevicename="/dev/i2c-1", I2Caddr=110); // I2Caddr is not in HEX-Format

  // Write (one byte) to register 0x02 (version 1)
  [sim] = ld_I2CDevice_Write(sim, 0, ObjectIdentifyer="LEDArray1", Register=2, L1_);
  
  // Raw write (one byte) a register (version 2)
  [sim, Addr] = ld_constvecInt32(sim, 0, 2);  // 0x02
  [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer="LEDArray1", in=Addr, vecsize=1); // one byte
  [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer="LEDArray1", in=L1_, vecsize=1);  // one byte
  [sim] = ld_I2CDevice_Transmit(sim, 0, ObjectIdentifyer="LEDArray1");  // transmit all bytes written by BufferWrite
  
  // Read out the register (one byte)
  [sim, out] = ld_I2CDevice_Read(sim, 0, ObjectIdentifyer="LEDArray1", Register=2)

  // print data
  [sim, out] = ld_Int32ToFloat(sim, 0, out);
  [sim] = ld_printf(sim, 0, out, "register = ", 1);




  outlist = list();
endfunction




// This is the main top level schematic
function [sim, outlist] = schematic_fn(sim, inlist)  

// 
// Create a thread that runs the control system
// 
   
        ThreadPrioStruct.prio1=ORTD.ORTD_RT_NORMALTASK; // or  ORTD.ORTD_RT_NORMALTASK
        ThreadPrioStruct.prio2=0; // for ORTD.ORTD_RT_REALTIMETASK: 1-99 as described in   man sched_setscheduler
                                  // for ORTD.ORTD_RT_NORMALTASK this is the nice-value (higher value means less priority)
        ThreadPrioStruct.cpu = -1; // The CPU on which the thread will run; -1 dynamically assigns to a CPU, 
                                   // counting of the CPUs starts at 0

        [sim, StartThread] = ld_initimpuls(sim, ev); // triggers your computation only once
        [sim, outlist, computation_finished] = ld_async_simulation(sim, ev, ...
                              inlist=list(), ...
                              insizes=[], outsizes=[], ...
                              intypes=[], outtypes=[], ...
                              nested_fn = Thread_MainRT, ...
                              TriggerSignal=StartThread, name="MainRealtimeThread", ...
                              ThreadPrioStruct, userdata=list() );
       

//    NOTE: for rt_preempt real-time you can use e.g. the following parameters:
// 
//         // Create a RT thread on CPU 0:
//         ThreadPrioStruct.prio1=ORTD.ORTD_RT_REALTIMETASK; // rt_preempt FIFO scheduler
//         ThreadPrioStruct.prio2=50; // Highest priority
//         ThreadPrioStruct.cpu = 0; // CPU 0


   // output of schematic (empty)
   outlist = list();
endfunction

  








//

  function sim=i2c_TLC59116_driver(sim, ObjectIdentifyer, I2CDevicename, I2Caddr, pwm)
    //     
    // This is a driver for the TLC59116 (16 channel pwm LED driver)
    // http://www.elv.de/I2C-Bus-LED-Treiber-Steuern-Sie-16-LEDs-%C3%BCber-nur-2-Leitungen/x.aspx/cid_726/detail_31520
    //     

    // open the device using the linux-kernel I2C driver interface
    [sim] = ld_I2CDevice_shObj(sim, 0, ObjectIdentifyer, Visibility=2, I2CDevicename, I2Caddr); 
    

    // Define some constants
    [sim, Hex_aa] = ld_constvecInt32(sim, 0, 170); // 0xaa 
    [sim, Hex_ff] = ld_constvecInt32(sim, 0, 255); // 0xff
    [sim, Hex_00] = ld_constvecInt32(sim, 0, 0); // 0x00

    // Initiate command
    [sim, Hex_80] = ld_constvecInt32(sim, 0, 128); // 0x80 start writing registers at register 0x00
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_80, vecsize=1);

    // Write to registers
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_00, vecsize=1);
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_00, vecsize=1);

    // write to pwm registers
    for i=1:16
      [sim,pwm_] = ld_floorInt32(sim, 0, in=pwm(i));  
      [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=pwm_, vecsize=1);
    end

    // write remaining registers
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_ff, vecsize=1);
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_00, vecsize=1);
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_aa, vecsize=1);
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_aa, vecsize=1);
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_aa, vecsize=1);
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_aa, vecsize=1);
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_00, vecsize=1);
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_00, vecsize=1);
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_00, vecsize=1);
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_00, vecsize=1);
    [sim] = ld_I2CDevice_BufferWrite(sim, 0, ObjectIdentifyer, in=Hex_ff, vecsize=1);

    // send everything
    [sim] = ld_I2CDevice_Transmit(sim, 0, ObjectIdentifyer);
  endfunction
// Set-up (no detailed understanding necessary)
//

thispath = get_absolute_file_path(ProgramName+'.sce');
cd(thispath);
z = poly(0,'z');

// defile ev
ev = [0]; // main event

// set-up schematic by calling the user defined function "schematic_fn"
insizes = []; outsizes=[];
[sim_container_irpar, sim]=libdyn_setup_schematic(schematic_fn, insizes, outsizes);

// pack the simulation into a irpar container
parlist = new_irparam_set();
parlist = new_irparam_container(parlist, sim_container_irpar, 901); // pack simulations into irpar container with id = 901
par = combine_irparam(parlist); // complete irparam set
save_irparam(par, ProgramName+'.ipar', ProgramName+'.rpar'); // Save the schematic to disk

// clear
par.ipar = []; par.rpar = [];






