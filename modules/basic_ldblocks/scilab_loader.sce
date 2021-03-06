


// 
// ortd_checkpar types:
// 
//     'Signal' 
//     'SignalList' 
//     'SingleValue' 
//     'Vector'
//     'String'
// 
//  e.g.
// 
//   ortd_checkpar(sim, list('Signal', 'in', in) );
//   ortd_checkpar(sim, list('SingleValue', 'gain', gain) );
// 



// TODO: FIXME: this is implemented twice. Remove the block below
// function [sim,out] = ld_getsign(sim, events, in) // PARSEDOCU_BLOCK
//   btype = 60001 + 26;


// 
// More basic functions that could also added to libdyn.sci
// 

function [sizes, types] = ld_getSizesAndTypes(sim, ev, SignalList)
//
// Tries to obtain the vector size and type of each signal given through the "SignalList"
//

    N = length(SignalList);
    sizes = zeros(N,1);
    types = zeros(N,1);

    for i=1:length(SignalList)
  //disp(i);
  try
      Signal = SignalList(i); // FIXME: This will only work for blocks, not system inputs, feed or other objects
      portN = sim.objectlist(Signal.oid).outport;
      sizes(i) = sim.objectlist(Signal.highleveloid).outsizes(portN+1);
      types(i) = sim.objectlist(Signal.highleveloid).outtypes(portN+1);
  catch
    printf("ld_getSizes\n");   
    error("ld_getSizesAndTypes: You have a source in SignalList, that doesn''t provide information about the vector size and datatype.");;
  end
    end
endfunction


// 
// More basic functions that could also added to libdyn.sci or so
// 



// Creates a Block that solely creates a globally shared object
function [sim] =  libdyn_CreateSharedObjBlk(sim, btype, ObjectIdentifyer, Visibility, Uipar, Urpar)
  
  events = 0;
  insizes=[]; // Input port sizes
  outsizes=[]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[]; // datatype for each input port
  outtypes=[]; // datatype for each output port

  blocktype = 2; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)


  // Create the block
   [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, ObjectIdentifyer, Visibility);
//   [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, 'test');

  // ensure the block is included in the simulation even without any I/O ports
 sim = libdyn_include_block(sim, blk);
  
  // end new fn (sim)
endfunction

// new version for this functoin
// V2
function [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed, varargin)
  [lhs,rhs]=argn(0);
// 
// Create a I/O Configuration for the block that can be read out by the libdyn_AutoConfigureBlock() - C function
// during block's configuration
// 

  // ObjectIdentifyer


  if length(insizes) ~= length(intypes) then
    error("length(insizes) ~= length(intypes)");
  end
  if length(outsizes) ~= length(outtypes) then
    error("length(outsizes) ~= length(outtypes)");
  end

  

  param = [blocktype];

  parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, insizes, 10); 
   parlist = new_irparam_elemet_ivec(parlist, outsizes, 11); 
   parlist = new_irparam_elemet_ivec(parlist, intypes, 12); 
   parlist = new_irparam_elemet_ivec(parlist, outtypes, 13); 
   parlist = new_irparam_elemet_ivec(parlist, dfeed, 14); 
   parlist = new_irparam_elemet_ivec(parlist, param, 15); 

   parlist = new_irparam_elemet_ivec(parlist, Uipar, 20); 
   parlist = new_irparam_elemet_rvec(parlist, Urpar, 21); 

   rhs=argn(2);
   if ( rhs > 11 ) then
     if (rhs == 13) then
       ObjectIdentifyer = varargin(1);
       Visibility = varargin(2);
       printf("Defining a Shared Object %s Visibility is %d\n", ObjectIdentifyer, Visibility);
       parlist = new_irparam_elemet_ivec(parlist, ascii(ObjectIdentifyer), 30); 
       parlist = new_irparam_elemet_ivec(parlist, Visibility, 31);     
     end
     if (rhs == 12) then
       ObjectIdentifyer = varargin(1);
       printf("Accessing a Shared Object %s\n", ObjectIdentifyer);
       parlist = new_irparam_elemet_ivec(parlist, ascii(ObjectIdentifyer), 30);        
     end
   end

   
   blockparam = combine_irparam(parlist);

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ blockparam.ipar  ], rpar=[ blockparam.rpar ], ...
                  insizes, outsizes, ...
                  intypes, outtypes );
endfunction




// V1
// function [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed)
// // 
// // Create a I/O Configuration for the block that can be read out by the libdyn_AutoConfigureBlock() - C function
// // during block's configuration
// // 
// 
//   // ObjectIdentifyer
// 
//   if length(insizes) ~= length(intypes) then
//     error("length(insizes) ~= length(intypes)");
//   end
//   if length(outsizes) ~= length(outtypes) then
//     error("length(outsizes) ~= length(outtypes)");
//   end
// 
//   
// 
//   param = [blocktype];
// 
//   parlist = new_irparam_set();
// 
//    parlist = new_irparam_elemet_ivec(parlist, insizes, 10); 
//    parlist = new_irparam_elemet_ivec(parlist, outsizes, 11); 
//    parlist = new_irparam_elemet_ivec(parlist, intypes, 12); 
//    parlist = new_irparam_elemet_ivec(parlist, outtypes, 13); 
//    parlist = new_irparam_elemet_ivec(parlist, dfeed, 14); 
//    parlist = new_irparam_elemet_ivec(parlist, param, 15); 
// 
//    parlist = new_irparam_elemet_ivec(parlist, Uipar, 20); 
//    parlist = new_irparam_elemet_rvec(parlist, Urpar, 21); 
// 
//    rhs=argn(2);
//    if ( rhs > 11 ) then
//      if (rhs == 13) then
//        printf("Defining a Shared Object %s Visibility is %d\n", ObjectIdentifyer, Visibility);
//        parlist = new_irparam_elemet_ivec(parlist, ascii(ObjectIdentifyer), 30); 
//        parlist = new_irparam_elemet_ivec(parlist, Visibility, 31);     
//      end
//      if (rhs == 12) then
//        printf("Accessing a Shared Object %s\n", ObjectIdentifyer);
//        parlist = new_irparam_elemet_ivec(parlist, ascii(ObjectIdentifyer), 30);        
//      end
//    end
// 
//    
//    blockparam = combine_irparam(parlist);
// 
//   [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ blockparam.ipar  ], rpar=[ blockparam.rpar ], ...
//                   insizes, outsizes, ...
//                   intypes, outtypes );
// 
// endfunction
// 









// 
// Interfacing functions are placed in this place
// 

function [sim] = ld_file_save_machine2(sim, ev, inlist, cntrl, FileNamesList) // PARSEDOCU_BLOCK
//
// %PURPOSE: Start and stop saving of multiple data vectors to multiple files
// 
//   inlist list() of *+ - Data to write
//   cntrl * - if cntrl steps to 2 then saving is started; if it steps to 1 saving is stopped
//   FileNamesList list() of strings - Filenames for saving
// 
// Note: This function tries to automatically detect the vector size for each entry of inlist.
//       Howver, this does not work for all signal sources (blocks) at the moment.
//       If come accross such a situation, you're invited to notify the authors of ORTD.
// 
// Note: The implementation of this function is a superblock using state machines
//       and the ld_savefile block. If you're invited to take a look at its source for a nice
//       example on using state machines.
// 
// 
// Example:
// 
//       TriggerSave = ...
// 
//       SaveSignals=list();        FileNamesList=list();
//       SaveSignals($+1) = Signal1;      FileNamesList($+1) = "measurements/Signal1.dat";
//       SaveSignals($+1) = Signal2;      FileNamesList($+1) = "measurements/Signal2.dat";
// 
//       [sim] = ld_file_save_machine2(sim, ev, ...
//                          inlist=SaveSignals, ...
//                          cntrl=TriggerSave, FileNamesList          );
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'cntrl', cntrl) );
  ortd_checkpar(sim, list('SignalList', 'inlist', inlist) );
end


    function [sim, outlist, active_state, x_global_kp1, userdata] = state_mainfn(sim, inlist, x_global, state, statename, userdata)
      // This function is called multiple times -- once for each state.
      // At runtime, these are three different nested simulations. Switching
      // between them represents state changing, thus each simulation 
      // represents a certain state.
      
//       printf("defining savemachine state %s (#%d) ... userdata(1)=%s\n", statename, state, userdata(1) );
      
      // define names for the first event in the simulation
      ev = 0; events = ev;

      DataLength = userdata(1);      NumPorts = userdata(2);    FileNames = userdata(3);
      // 

      switch = inlist(NumPorts+1);  [sim, switch] = ld_gain(sim, ev, switch, 1);


      // demultiplex x_global
      [sim, x_global] = ld_demux(sim, events, vecsize=1, invec=x_global);


      // The signals "active_state" is used to indicate state switching: A value > 0 means the 
      // the state enumed by "active_state" shall be activated in the next time step.
      // A value less or equal to zero causes the statemachine to stay in its currently active
      // state

      select state
  case 1 // state 1
    active_state = switch;
//    [sim] = ld_printf(sim, ev, in=dataToSave, str="Pauseing Save", insize=DataLen);

  case 2 // state 2
      for i=1:NumPorts  // for each port a write to file block
        dataToSave = inlist(i);
      [sim] = ld_savefile(sim, ev, FileNames(i), source=dataToSave, vlen=DataLength(i) );
     // [sim] = ld_printf(sim, ev, in=dataToSave, str="Saveing port "+string(i)+" ", insize=DataLength(i) );
      end

    active_state = switch;
      end

      // multiplex the new global states
      [sim, x_global_kp1] = ld_mux(sim, ev, vecsize=1, inlist=x_global);
      
      // the user defined output signals of this nested simulation
      outlist = list();
  endfunction



//  if length(insizes) ~= length(intypes) then
//    error("ld_file_save_machine2: length(insizes) ~= length(intypes)");
//  end
//  if length(insizes) ~= length(inlist) then
//    error("ld_file_save_machine2: length(insizes) ~= length(inlist)");
//  end
  if length(inlist) ~= length(FileNamesList) then
    error("ld_file_save_machine2: length(insizes) ~= length(FileNamesList)");
  end


//  DataLength = insizes;
  NumPorts = length(inlist);


//   [sim] = ld_printf(sim, ev, cntrl, "cntrl", 1);

  [sim, cntrl] = ld_detect_step_event(sim, ev, in=cntrl, eps=0.2);

//   [sim] = ld_printf(sim, ev, cntrl, "Dcntrl", 1);

  Cinlist = list();
  for i=1:NumPorts
    Cinlist(i) = inlist(i);
  end
  Cinlist(NumPorts+1) = cntrl;

  // get the types and the sizes of the given signals
  [sizes, types] = ld_getSizesAndTypes(sim, ev, SignalList=Cinlist);

  DataLength = sizes;

  // set-up two states represented by two nested simulations
  [sim, outlist, x_global, active_state,userdata] = ld_statemachine(sim, ev=0, ...
      inlist=Cinlist, ..
      insizes=[sizes(:)' ], outsizes=[], ... 
      intypes=[types(:)' ], outtypes=[], ...
      nested_fn=state_mainfn, Nstates=2, state_names_list=list("pause", "save"), ...
      inittial_state=1, x0_global=[1], userdata=list(DataLength, NumPorts, FileNamesList)  );


endfunction


function [sim] = ld_MultiFileSave(sim, ev, inlist, cntrl, FileNamesList) // PARSEDOCU_BLOCK
//
// %PURPOSE: Start and stop saving of multiple data vectors to multiple files
// 
//   inlist list() of *+ - Data to write
//   cntrl * - if cntrl steps to 2 then saving is started; if it steps to 1 saving is stopped
//   FileNamesList list() of strings - Filenames for saving
// 
// Note: This function tries to automatically detect the vector size for each entry of inlist.
//       Howver, this does not work for all signal sources (blocks) at the moment.
//       If come accross such a situation, you're invited to notify the authors of ORTD.
// 
// Note: cntrl does not have an effect by now
// 
// Example:
// 
//       TriggerSave = ...
// 
//       SaveSignals=list();        FileNamesList=list();
//       SaveSignals($+1) = Signal1;      FileNamesList($+1) = "measurements/Signal1.dat";
//       SaveSignals($+1) = Signal2;      FileNamesList($+1) = "measurements/Signal2.dat";
// 
//       [sim] = ld_MultiFileSave(sim, 0, ...
//                          inlist=SaveSignals, ...
//                          cntrl=TriggerSave, FileNamesList          );
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'cntrl', cntrl) );
  ortd_checkpar(sim, list('SignalList', 'inlist', inlist) );
end




  if length(inlist) ~= length(FileNamesList) then
    error("ld_file_save_machine2: length(insizes) ~= length(FileNamesList)");
  end


  NumPorts = length(inlist);

  // get the types and the sizes of the given signals
  [sizes, types] = ld_getSizesAndTypes(sim, 0, SignalList=inlist);
  
  for i=1:NumPorts  // for each port a write to file block

       dataToSave = inlist(i);
      [sim] = ld_savefile(sim, 0, FileNamesList(i), source=dataToSave, vlen=sizes(i) );

  end
  
 
endfunction



function [sim] = ld_savefile(sim, events, fname, source, vlen) // PARSEDOCU_BLOCK
//
// %PURPOSE: Quick and easy dumping of signals to files
// 
// fname - string of the filename
// source *+ - source signal
// vlen - vector size of signal
// 


if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'source', source) );
  ortd_checkpar(sim, list('SingleValue', 'vlen', vlen) );
  ortd_checkpar(sim, list('String', 'fname', fname) );
end

  [inp] = libdyn_extrakt_obj( source ); // compatibility

  autostart = 1;
  maxlen = 0
  fname = ascii(fname);

  btype = 130;

  [sim,blk] = libdyn_new_block(sim, events, btype, [maxlen, autostart, vlen, length(fname), fname(:)'], [],  ...
          insizes=[ vlen ], outsizes=[], ...
          intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[]  );
  
  [sim,save_] = libdyn_conn_equation(sim, blk, list(source) );
endfunction


function [sim, out] = ld_switch2to1(sim, events, cntrl, in1, in2) // PARSEDOCU_BLOCK
//
// %PURPOSE: A 2 to 1 switching Block
//
// cntr * - control input
// in1 *
// in2 *
// out * - output
//
// if cntrl > (greather than) 0 : out = in1
// if cntrl < (smaller than) 0 : out = in2
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'cntrl', cntrl) );
  ortd_checkpar(sim, list('Signal', 'in1', in1) );
  ortd_checkpar(sim, list('Signal', 'in2', in2) );
end


  btype = 60001;
  [sim,blk] = libdyn_new_block(sim, events, btype, [], [], ...
                   insizes=[1, 1, 1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], ...
                   outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(cntrl, in1, in2) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction





function [sim, outlist] = ld_demuxInt32(sim, events, vecsize, invec) // PARSEDOCU_BLOCK
//
// %PURPOSE: Demultiplexer int 32
//
// invec * - input vector signal to be split up
// outlist *LIST - list() of output signals
//
//
// Splits the input vector signal "invec" of size "vecsize" up into 
//
// outlist(1)
// outlist(2)
//  ....
//    

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'invec', invec) );
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
end

  btype = 60001 + 37;  
  ipar = [vecsize, 0]; rpar = [];
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[vecsize], outsizes=[ones(vecsize,1)], ...
                       intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32*ones(vecsize,1)]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(invec) );

  // connect each outport
  outlist = list();
  for i = 1:vecsize
    [sim,out] = libdyn_new_oport_hint(sim, blk, i-1);   // ith port
    outlist(i) = out;
  end
endfunction


function [sim, outlist] = ld_demux(sim, events, vecsize, invec) // PARSEDOCU_BLOCK
//
// %PURPOSE: Demultiplexer
//
// invec * - input vector signal to be split up
// outlist *LIST - list() of output signals
//
//
// Splits the input vector signal "invec" of size "vecsize" up into 
//
// outlist(1)
// outlist(2)
//  ....
//    

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'invec', invec) );
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
end

  btype = 60001 + 1;  
  ipar = [vecsize, 0]; rpar = [];
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[vecsize], outsizes=[ones(vecsize,1)], ...
                       intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT*ones(vecsize,1)]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(invec) );

  // connect each outport
  outlist = list();
  for i = 1:vecsize
    [sim,out] = libdyn_new_oport_hint(sim, blk, i-1);   // ith port
    outlist(i) = out;
  end
endfunction


function [sim, out] = ld_mux(sim, events, vecsize, inlist) // PARSEDOCU_BLOCK
//    
// %PURPOSE: Multiplexer
//
// inlist *LIST - list() of input signals of size 1
// out *+ - output vector signal
// 
// 
// combines inlist(1), inlist(2), ...    
// to a vector signal "out" of size "vecsize", whereby each inlist(i) is of size 1
//    

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('SignalList', 'inlist', inlist) );
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
end

  btype = 60001 + 2;  
  ipar = [vecsize; 0]; rpar = [];

  if (length(inlist) ~= vecsize) then
    printf("Incorect number of input ports to ld_mux. %d != %d\n", length(inlist), vecsize );
    error(".");
  end

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[ones(1,vecsize)], outsizes=[vecsize], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT*ones(1,vecsize) ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, inlist );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_muxInt32(sim, events, vecsize, inlist) // PARSEDOCU_BLOCK
//    
// %PURPOSE: Multiplexer int 32
//
// inlist *LIST - list() of input signals of size 1
// out *+ - output vector signal
// 
// 
// combines inlist(1), inlist(2), ...    
// to a vector signal "out" of size "vecsize", whereby each inlist(i) is of size 1
//    

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('SignalList', 'inlist', inlist) );
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
end

  btype = 60001 + 38;  
  ipar = [vecsize; 0]; rpar = [];

  if (length(inlist) ~= vecsize) then
    printf("Incorect number of input ports to ld_mux. %d != %d\n", length(inlist), vecsize );
    error(".");
  end

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[ones(1,vecsize)], outsizes=[vecsize], ...
                                     intypes=[ ORTD.DATATYPE_INT32*ones(1,vecsize) ], outtypes=[ORTD.DATATYPE_INT32] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, inlist );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_hysteresis(sim, events, in, switch_on_level, switch_off_level, initial_state, onout, offout) // PARSEDOCU_BLOCK
// %PURPOSE: hysteresis block
//
// in * - input
// out * -output
// 
// switches out between onout and offout
// initial state is either -1 (off) or 1 (on)
//
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'switch_off_level', switch_off_level) );
  ortd_checkpar(sim, list('SingleValue', 'switch_on_level', switch_on_level) );
  ortd_checkpar(sim, list('SingleValue', 'onout', onout) );
  ortd_checkpar(sim, list('SingleValue', 'offout', offout) );
end

  if (switch_off_level > switch_on_level) then
    error("ld_hysteresis: setting switch_off_level > switch_on_level makes no sense\n");
  end

  btype = 60001 + 3;
  [sim,blk] = libdyn_new_block(sim, events, btype, [initial_state], [ switch_on_level, switch_off_level, onout, offout] , ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_modcounter(sim, events, in, initial_count, mod) // PARSEDOCU_BLOCK
// %PURPOSE: Modulo Counter - Block
//
// in * - input
// out * -output
// 
// A counter that increases its value for each timestep for which in > 0 is true.
// if the counter value >= mod then it is reset to counter = initial_count
//
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'initial_count', initial_count) );
  ortd_checkpar(sim, list('SingleValue', 'mod', mod) );
end

  if (mod < 0) then
    error("ld_modcounter: mod is less than zero\n");
  end

  btype = 60001 + 4;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ initial_count, mod ], [  ],  ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]   );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_jumper(sim, events, in, steps) // PARSEDOCU_BLOCK
// %PURPOSE: jumper - block
//
// out *+ - vector of size steps
// in * - switching input
//
// The vector out always contains one "1", the rest is zero.
// The "1" moves to the right if in > 0. If the end is reached
// it "1" flips back to the left side
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'steps', steps) );
end

  if (steps <= 0) then
    error("ld_jumper: steps must be greater than zero\n");
  end

  btype = 60001 + 5;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ steps ], [  ], ...
                   insizes=[1], outsizes=[ steps ], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_memory(sim, events, in, rememberin, initial_state) // PARSEDOCU_BLOCK
// %PURPOSE: memory - block
//
// in * - input
// rememberin * - 
// out * - output
// 
// If rememberin > 0 then
//   remember in, which is then feed to the output out until it is overwritten by a new value
//
// Please note that input ist applied to the output immediately (without a delay)
//
// initial output out = initial_state
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('Signal', 'rememberin', rememberin) );
  ortd_checkpar(sim, list('SingleValue', 'initial_state', initial_state) );
end


  memsize = length(initial_state);

  btype = 60001 + 6;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ memsize  ], [ initial_state ],  ...
                   insizes=[memsize, 1], outsizes=[memsize], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]   );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in, rememberin) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_abs(sim, events, in) // PARSEDOCU_BLOCK
// %PURPOSE: abs - block
//
// in * - input
// out * - output
// 
// out = abs(in)
// 


if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
end

  btype = 60001 + 7;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_extract_element(sim, events, invec, pointer, vecsize ) // PARSEDOCU_BLOCK
  //
  // %PURPOSE: Extract one element of a vector
  //
  // invec *+ - the input vector signal
  // pointer * - the index signal (indexing starts at 1)
  // vecsize - length of input vector
  // 
  // out = invec[pointer], the first element is at pointer = 1
  //

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'invec', invec) );
  ortd_checkpar(sim, list('Signal', 'pointer', pointer) );
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
end

  btype = 60001 + 8;  
  ipar = [ vecsize, ORTD.DATATYPE_FLOAT ]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[vecsize, 1], outsizes=[1], ...
                       intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

//   [sim,blk] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar               );


  [sim,blk] = libdyn_conn_equation(sim, blk, list(invec, pointer) );

  // connect each outport
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // ith port
endfunction


function [sim, out] = ld_constvec(sim, events, vec) // PARSEDOCU_BLOCK
// 
// %PURPOSE: a constant vector
// 
// out *+ - the vector
// 


  btype = 60001 + 9;  
  ipar = [length(vec); 0]; rpar = [vec];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                   insizes=[], outsizes=[ length(vec) ], ...
                   intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]  );
 
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



function [sim, out] = ld_counter(sim, events, count, reset, resetto, initial) // PARSEDOCU_BLOCK
// 
// %PURPOSE: A resetable counter block
//
// count * - signal
// reset * - signal
// resetto * - signal
// initial - constant
// out * - output
// 
// increases out by count (out = out + count)
// 
// if reset > 0.5 then
//   out = resetto
//
// initially out is set to initial
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'count', count) );
  ortd_checkpar(sim, list('Signal', 'reset', reset) );  
  ortd_checkpar(sim, list('Signal', 'resetto', resetto) );
  ortd_checkpar(sim, list('SingleValue', 'initial', initial) );
end

  btype = 60001 + 10;
  ipar = [  ]; rpar = [ initial ];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[1,1,1], outsizes=[1], ...
                       intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], ...
                       outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list( count, reset, resetto ) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_shift_register(sim, events, in, len) // FIXME TODO
// %PURPOSE: A shift register with access to the stored values
//
// in * - will be put to the first position in the register that was shifted before.
// out *+(len) - the whole memory
//    
// 
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'len', len) );
end

  btype = 60001 + 11;
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[ 1 ], outsizes=[len], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction






// Lookup table: Inputs between lowerin and upperin will be mapped linear to the indices of table
//               The corresponsing element of table will be the output
function [sim,bid] = libdyn_new_blk_lkup(sim, events, lowerin, upperin, table)
  btype = 120;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [length(table)], [ lowerin, upperin, table(:)' ]);
endfunction
function [sim, out] = ld_lookup(sim, events, u, lower_b, upper_b, table, interpolation) // PARSEDOCU_BLOCK
// %PURPOSE: Lookup table - block
//
// in * - input
// out * - output
// 
// 
// lower_b - smallest value of the input signal to map to the table
// upper_b - biggest value of the input signal to map to the table
// table - the table (Scilab vector)
// 
// Mapping is done in a linear way:
//   out = table( (in - lowerin) / (upperin - lowerin) )
// 
// interpolation = 0 : no interpolation
// interpolation = 1 : linear interpolation
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'u', u) );
  ortd_checkpar(sim, list('SingleValue', 'lower_b', lower_b) );
  ortd_checkpar(sim, list('SingleValue', 'upper_b', upper_b) );

  ortd_checkpar(sim, list('SingleValue', 'interpolation', interpolation) );
end

  btype = 60001 + 12;
  [sim,blk] = libdyn_new_block(sim, events, btype, [length(table), interpolation ], [ lower_b, upper_b, table(:)' ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(u) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction





function [sim, out] = ld_vector_lookup(sim, events, u, lower_b, upper_b, table, interpolation, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: Lookup table - block
//
// in *+(vecsize) - input
// out *+(vecsize) - output
// 
// 
// lower_b - smallest value of the input signal to map to the table
// upper_b - biggest value of the input signal to map to the table
// table - the table (Scilab vector)
// 
// Mapping is done in a linear way:
//   out = table( (in - lowerin) / (upperin - lowerin) )
// 
// interpolation = 0 : no interpolation
// interpolation = 1 : linear interpolation
// 
// 

  if ORTD.FASTCOMPILE==%f then
    ortd_checkpar(sim, list('Signal', 'u', u) );
    ortd_checkpar(sim, list('SingleValue', 'lower_b', lower_b) );
    ortd_checkpar(sim, list('SingleValue', 'upper_b', upper_b) );

    ortd_checkpar(sim, list('SingleValue', 'interpolation', interpolation) );
  end

  btype = 60001 + 72;
  [sim,blk] = libdyn_new_block(sim, events, btype, [length(table), interpolation, vecsize ], [ lower_b, upper_b, table(:)' ], ...
                   insizes=[ vecsize ], outsizes=[ vecsize ], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(u) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction




function [sim, out] = ld_not(sim, events, in) // PARSEDOCU_BLOCK
// %PURPOSE: logic negation - block
//
// in * - input
// out * - output
// 
// out = 0, if in > 0.5  OR  out = 1, if in < 0.5
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
end

  btype = 60001 + 13;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_or(sim, events, inlist) // PARSEDOCU_BLOCK
// %PURPOSE: logic or - block
//
// in *LIST - list() of inputs (for now the exactly two inputs are possible)
// out * - output
// 
// 
// 

//if ORTD.FASTCOMPILE==%f then
//  ortd_checkpar(sim, list('Signal', 'in', in) );
//end

  Nin=length(inlist);

  if (Nin ~= 2) then
    error("invalid number of inputs");
  end

  insizes=ones(1, Nin);
  intypes=ones(1, Nin) * ORTD.DATATYPE_FLOAT;

  btype = 60001 + 14;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[  ], rpar=[   ], ...
                   insizes, outsizes=[1], ...
                   intypes, outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list( inlist(1), inlist(2) ) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_iszero(sim, events, in, eps) // PARSEDOCU_BLOCK
//
// %PURPOSE: check if input is near zero
//
// in * - input
// out * - output
// 
// out = 1, if in between -eps and eps, othwewise out = 0
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'eps', eps) );
end

  btype = 60001 + 15;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[  ], rpar=[ eps ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_limitedcounter(sim, events, count, reset, resetto, initial, lower_b, upper_b) // PARSEDOCU_BLOCK
// 
// %PURPOSE: A resetable, limited counter block
//
// count * - signal
// reset * - signal
// resetto * - signal
// initial - constant
// out * - output
// 
// increases out by count (out = out + count), but count is always between lower_b and upper_b
// 
// if reset > 0.5 then
//   out = resetto
//
// initially out is set to initial
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'count', count) );
  ortd_checkpar(sim, list('Signal', 'reset', reset) );
  ortd_checkpar(sim, list('Signal', 'resetto', resetto) );

  ortd_checkpar(sim, list('SingleValue', 'initial', initial) );
  ortd_checkpar(sim, list('SingleValue', 'lower_b', lower_b) );
  ortd_checkpar(sim, list('SingleValue', 'upper_b', upper_b) );
end

  if (lower_b > upper_b) then
    error("lower_b is greater than upper_b");
  end

  btype = 60001 + 16;
  ipar = [  ]; rpar = [ initial, lower_b, upper_b ];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[1,1,1], outsizes=[1], ...
                       intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], ...
                       outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list( count, reset, resetto ) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_memorydel(sim, events, in, rememberin, initial_state) // PARSEDOCU_BLOCK
// %PURPOSE: delayed memory - block
//
// in * - input
// rememberin * - 
// out * - output
// 
// If rememberin > 0 then
//   remember in, which is then feed to the output "out" in the next time step until it is overwritten by a new value
//
// initial output out = initial_state
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('Signal', 'rememberin', rememberin) );
  ortd_checkpar(sim, list('SingleValue', 'initial_state', initial_state) );
end


  memsize = length(initial_state);

  btype = 60001 + 17;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ memsize  ], [ initial_state ],  ...
                   insizes=[memsize, 1], outsizes=[memsize], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]   );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in, rememberin) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_steps(sim, events, activation_simsteps, values) // PARSEDOCU_BLOCK
//
// %PURPOSE: steps
//
// out * - output
// 
// 

//   ortd_checkpar(sim, list('Signal', 'in', in) );
//   ortd_checkpar(sim, list('SingleValue', 'eps', eps) );


  if (length(activation_simsteps) ~= length(values)-1) then
    error("length(activation_simsteps) != length(values)-1");
  end

  btype = 60001 + 18;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ length(values), activation_simsteps ], rpar=[ values ], ...
                   insizes=[], outsizes=[1], ...
                   intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]  );


  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_cond_overwrite(sim, events, in, condition, setto) // PARSEDOCU_BLOCK
//
// %PURPOSE: conditional overwrite of the input signal's value
//
// out * - output
// in * - input to potentially overwrite
// condition * - condition signal
// 
// out = in, if condition < 0.5
// out = setto, otherwise
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'setto', setto) );
  ortd_checkpar(sim, list('Signal', 'condition', condition) );
end

  btype = 60001 + 19;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ ], rpar=[ setto ], ...
                   insizes=[1,1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(condition, in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_cond_overwrite2(sim, events, in, condition, setto) // PARSEDOCU_BLOCK
//
// %PURPOSE: conditional overwrite of the input signal's value
//
// out * - output (float)
// in * - input (float) to potentially overwrite
// condition * - condition signal (int32) -- in contrast to ld_cond_overwrite
// 
// out = in, if condition > 0
// out = setto, otherwise
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'setto', setto) );
  ortd_checkpar(sim, list('Signal', 'condition', condition) );
end

  btype = 60001 + 43;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ ], rpar=[ setto ], ...
                   insizes=[1,1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(condition, in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_cond_overwriteInt32(sim, events, in, condition, setto) // PARSEDOCU_BLOCK
//
// %PURPOSE: conditional overwrite of the input signal's value
//
// out * - output (int32)
// in * - input (int32) to potentially overwrite
// condition * - condition signal (int32) -- in contrast to ld_cond_overwrite
// 
// out = in, if condition > 0
// out = setto, otherwise
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'setto', setto) );
  ortd_checkpar(sim, list('Signal', 'condition', condition) );
end

  btype = 60001 + 49;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ setto ], rpar=[  ], ...
                   insizes=[1,1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(condition, in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction




function [sim, out] = ld_ramp(sim, events, in_from, in_to, start, reset, ramp_duration) // NOT FINISHED
//
// %PURPOSE: Online configurable ramp block
//
// out * - output (from 0 to 1)
// start * - if > 0.5 the ramp starts
// reset * - if > 0.5 the blocks states are reset
// increase * - constant by which the output is increased for each time step
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'start', start) );
  ortd_checkpar(sim, list('Signal', 'reset', reset) );
  ortd_checkpar(sim, list('Signal', 'increase', increase) );
end

  btype = 60001 + 20;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ ], rpar=[  ], ...
                   insizes=[1,1,1], outsizes=[1], ...
                   intypes=ORTD.DATATYPE_FLOAT*[1,1,1], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(  start, reset, increase ) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_and(sim, events, inlist) // PARSEDOCU_BLOCK
// %PURPOSE: logic and - block
//
// in *LIST - list() of inputs (for now the exactly two inputs are possible)
// out * - output
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('SignalList', 'inlist', inlist) );
end

  Nin=length(inlist);

  if (Nin ~= 2) then
    error("invalid number of inputs");
  end

  insizes=ones(1, Nin);
  intypes=ones(1, Nin) * ORTD.DATATYPE_FLOAT;

  btype = 60001 + 21;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[  ], rpar=[   ], ...
                   insizes, outsizes=[1], ...
                   intypes, outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list( inlist(1), inlist(2) ) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_notInt32(sim, events, in) // PARSEDOCU_BLOCK
// %PURPOSE: logic negation - block
//
// in * - input
// out * - output
// 
// out = 0, if in >= 1  OR  out = 1, if in < 1
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
end

  btype = 60001 + 46;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_andInt32(sim, events, inlist) // PARSEDOCU_BLOCK
// %PURPOSE: logic and - block
//
// in *LIST - list() of inputs (for now the exactly two inputs are possible)
// out * - output
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('SignalList', 'inlist', inlist) );
end

  Nin=length(inlist);

  if (Nin ~= 2) then
    error("invalid number of inputs");
  end

  insizes=ones(1, Nin);
  intypes=ones(1, Nin) * ORTD.DATATYPE_INT32;

  btype = 60001 + 44;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[  ], rpar=[   ], ...
                   insizes, outsizes=[1], ...
                   intypes, outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list( inlist(1), inlist(2) ) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_SetBitsInt32(sim, events, in, BitPattern, BitNrStart, NumBits) // PARSEDOCU_BLOCK
// %PURPOSE: Copy a bit-pattern to the input
//
// in *(INT32) - input
// BitPattern *(INT32)
// out *(INT32) - output
//
// BitNrStart - position in the input at which to start copying bits. Counting starts at zero
// NUmBits - number of bits to copy
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'inlist', in) );
  ortd_checkpar(sim, list('Signal', 'inlist', BitPattern) );

  ortd_checkpar(sim, list('SingleValue', 'inlist', BitNrStart) );
  ortd_checkpar(sim, list('SingleValue', 'inlist', NumBits) );
end




  btype = 60001 + 94;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ BitNrStart,NumBits ], rpar=[   ], ...
                   insizes=[1,1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32,ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list( in, BitPattern ) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction
function [sim, out] = ld_GetBitsInt32(sim, events, in, BitNrStart, NumBits) // PARSEDOCU_BLOCK
// %PURPOSE: Copy a range of bits from the input to the output
//
// in *(INT32) - input
// out *(INT32) - output
//
// BitNrStart - position in the input at which to start copying bits. Counting starts at zero
// NUmBits - number of bits to copy
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'inlist', in) );
 
  ortd_checkpar(sim, list('SingleValue', 'inlist', BitNrStart) );
  ortd_checkpar(sim, list('SingleValue', 'inlist', NumBits) );
end




  btype = 60001 + 95;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ BitNrStart,NumBits ], rpar=[   ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction





function [sim, out] = ld_orInt32(sim, events, inlist) // PARSEDOCU_BLOCK
// %PURPOSE: logic or - block
//
// in *LIST - list() of inputs (for now the exactly two inputs are possible)
// out * - output
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('SignalList', 'inlist', inlist) );
end

  Nin=length(inlist);

  if (Nin ~= 2) then
    error("invalid number of inputs");
  end

  insizes=ones(1, Nin);
  intypes=ones(1, Nin) * ORTD.DATATYPE_INT32;

  btype = 60001 + 45;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[  ], rpar=[   ], ...
                   insizes, outsizes=[1], ...
                   intypes, outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list( inlist(1), inlist(2) ) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_initimpuls(sim, events) // PARSEDOCU_BLOCK
//
// %PURPOSE: initial impuls
//
// out * - output
// 
// 


  btype = 60001 + 22;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ ], rpar=[ ], ...
                   insizes=[], outsizes=[1], ...
                   intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]  );


  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim] = ld_printfstderr(sim, events, in, str, insize) // PARSEDOCU_BLOCK
//
// %PURPOSE: Print data to stderr (the console)
//
// in *+(insize) - vectorial input signal
//
// str is a string that is printed followed by the signal vector in
// of size insize
//
  //[sim,blk] = libdyn_new_printf(sim, events, str, insize);

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('String', 'str', str) );
  ortd_checkpar(sim, list('SingleValue', 'insize', insize) );
end

  btype = 60001 + 23;;
  str = ascii(str);
//   [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [insize, length(str), str(:)'], []);

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ insize, length(str), str(:)' ], rpar=[ ], ...
                   insizes=[ insize ], outsizes=[], ...
                   intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
endfunction

function [sim] = ld_printfstderr2(sim, events, in, str, insize) // PARSEDOCU_BLOCK
//
// %PURPOSE: Print str to stderr (the console)
//
// in *+(insize) - vectorial input signal
//
// str is a string that is printed followed by the signal vector in
// of size insize
//
  //[sim,blk] = libdyn_new_printf(sim, events, str, insize);

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('String', 'str', str) );
  ortd_checkpar(sim, list('SingleValue', 'insize', insize) );
end

  btype = 60001 + 36;;
  str = ascii(str);
//   [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [insize, length(str), str(:)'], []);

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ insize, length(str), str(:)' ], rpar=[ ], ...
                   insizes=[ insize ], outsizes=[], ...
                   intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
endfunction

function [sim] = ld_printfbar(sim, events, in, str) // PARSEDOCU_BLOCK
//
// %PURPOSE: Print a bar (the console)
//
// in *+(1) - vectorial input signal
//
// str is a string that is printed followed by a bar whose length depends on in
//
  
if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('String', 'str', str) );
end

  btype = 60001 + 29;
  str = ascii(str);
  insize = 1;
//   [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [insize, length(str), str(:)'], []);

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ insize, length(str), str(:)' ], rpar=[ ], ...
                   insizes=[ insize ], outsizes=[], ...
                   intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
endfunction


function [sim, out] = ld_delay(sim, events, u, N) // PARSEDOCU_BLOCK
// %PURPOSE: delay - block
//
// in * - input
// out * - output
// 
// delay in by N steps
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'u', u) );
  ortd_checkpar(sim, list('SingleValue', 'N', N) );
end

  if length(N) ~= 1 then
    error("N is not a scalar\n");
  end

  if (N < 1) then
    error("invalid delay");
  end

  btype = 60001 + 24;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ N ], [ ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(u) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



function [sim, out] = ld_steps2(sim, events, activation_simsteps, values) // PARSEDOCU_BLOCK
//
// %PURPOSE: steps, counter is increased on event, which is different to ld_steps
//
// out * - output
// 
// 

  if (length(activation_simsteps) ~= length(values)-1) then
    error("length(activation_simsteps) != length(values)-1");
  end

  btype = 60001 + 25;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ length(values), activation_simsteps ], rpar=[ values ], ...
                   insizes=[], outsizes=[1], ...
                   intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]  );


  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction




// function [sim,out] = ld_getsign(sim, events, in) // PARSEDOCU_BLOCK
// //
// // %PURPOSE: return the sign of the input sigal
// // either 1 or -1
// //
// 
//   ortd_checkpar(sim, list('Signal', 'in', in) );
// 
// 
//   btype = 60001 + 26;
//   [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
//                    insizes=[1], outsizes=[1], ...
//                    intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );
// 
//   [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
//   [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
// endfunction


function [sim, out] = ld_insert_element(sim, events, in, pointer, vecsize ) // PARSEDOCU_BLOCK
  //
  // %PURPOSE: Insert one element into a vector
  //
  // in *+ - the input element signal
  // pointer * - the index signal
  // vecsize - length of output vector
  // 
  // out[pointer] = in, the first element is at pointer = 1
  //

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('Signal', 'pointer', pointer) );
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
end

  btype = 60001 + 27; 
  ipar = [ vecsize, ORTD.DATATYPE_FLOAT ]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[1, 1], outsizes=[vecsize], ...
                       intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in, pointer) );

  // connect each outport
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // ith port
endfunction

function [sim] = ld_FlagProbe(sim, events, in, str, insize) // PARSEDOCU_BLOCK
//
// %PURPOSE: Print data and Flags (calc output, update states, reset states) to stderr (the console)
//
// in *+(insize) - vectorial input signal
//
// str is a string that is printed followed by the signal vector in
// of size insize
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('String', 'str', str) );
  ortd_checkpar(sim, list('SingleValue', 'insize', insize) );
end

  //[sim,blk] = libdyn_new_printf(sim, events, str, insize);
  btype = 60001 + 28;
  str = ascii(str);
//   [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [insize, length(str), str(:)'], []);

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ insize, length(str), str(:)' ], rpar=[ ], ...
                   insizes=[ insize ], outsizes=[], ...
                   intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
endfunction

function [sim,out] = ld_ceilInt32(sim, events, in) // PARSEDOCU_BLOCK
//
// %PURPOSE: ceil(in)
// 
// return value is of type ORTD.DATATYPE_INT32
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
end

  btype = 60001 + 30;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_Int32ToFloat(sim, events, in) // PARSEDOCU_BLOCK
//
// %PURPOSE: Convert int32 to double
// 
// ORTD.DATATYPE_INT32 --> ORTD.DATATYPE_FLOAT
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
end

  btype = 60001 + 31;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_vector_Int32ToFloat(sim, events, in, vecsize) // PARSEDOCU_BLOCK
//
// %PURPOSE: Convert int32 to double
// 
// ORTD.DATATYPE_INT32 --> ORTD.DATATYPE_FLOAT
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
end

  btype = 60001 + 77;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ vecsize ], [  ], ...
                   insizes=[ vecsize ], outsizes=[ vecsize ], ...
                   intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_floorInt32(sim, events, in) // PARSEDOCU_BLOCK
//
// %PURPOSE: ceil(in)
// 
// return value is of type ORTD.DATATYPE_INT32
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
end

  btype = 60001 + 32;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_vector_floorInt32(sim, events, in, vecsize) // PARSEDOCU_BLOCK
//
// %PURPOSE: ceil(in)
// 
// return value is of type ORTD.DATATYPE_INT32
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
end

  btype = 60001 + 73;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ vecsize ], [  ], ...
                   insizes=[ vecsize ], outsizes=[ vecsize ], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_roundInt32(sim, events, in) // PARSEDOCU_BLOCK
//
// %PURPOSE: ceil(in)
// 
// return value is of type ORTD.DATATYPE_INT32
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
end

  btype = 60001 + 33;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_constvecInt32(sim, events, vec) // PARSEDOCU_BLOCK
// 
// %PURPOSE: a constant vector of ORTD.DATATYPE_INT32
// 
// out *+ - the vector of int32
// 
  btype = 60001 + 34; 
  ipar = [length(vec); 0; vec]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                   insizes=[], outsizes=[ length(vec) ], ...
                   intypes=[], outtypes=[ORTD.DATATYPE_INT32]  );
 
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



function [sim,out] = ld_sumInt32(sim, events, in1, in2) // PARSEDOCU_BLOCK
//
// %PURPOSE: return sum of the input signals
// TODO
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in1', in1) );
  ortd_checkpar(sim, list('Signal', 'in2', in2) );
end

  btype = 60001 + 35;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1,1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in1, in2) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_getsign(sim, events, in) // PARSEDOCU_BLOCK
//
// %PURPOSE: return - in
// TODO
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
end

  btype = 60001 + 37;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_OneStepDelInt32(sim, events, in, init_state) // PARSEDOCU_BLOCK
//
// %PURPOSE: One time step delay for Int32
// TODO
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'init_state', init_state) );
end

  btype = 60001 + 38;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ init_state ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_MulInt32(sim, ev, in1, in2) // PARSEDOCU_BLOCK
//
// %PURPOSE: return multiplication of the input signals
// TODO
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in1', in1) );
  ortd_checkpar(sim, list('Signal', 'in2', in2) );
end

  btype = 60001 + 39;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1,1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in1, in2) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_DivInt32(sim, ev, num, den) // PARSEDOCU_BLOCK
//
// %PURPOSE: return num DIV den
// TODO. not implemented by now
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'num', num) );
  ortd_checkpar(sim, list('Signal', 'den', den) );
end

  btype = 60001 + 40;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1,1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(num, den) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_ModInt32(sim, ev, num, den) // PARSEDOCU_BLOCK
//
// %PURPOSE: return num MODULO den
// TODO: not implemented by now
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'num', num) );
  ortd_checkpar(sim, list('Signal', 'den', den) );
end

  btype = 60001 + 41;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1,1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(num, den) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_CompareEqInt32(sim, events, in, CompVal) // PARSEDOCU_BLOCK
//
// %PURPOSE: Compeare to CompVal (if equal)
// 
// in*, float
// out*, int32 - 0 if (in == CompVal); 1 if (in != CompVal);
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'CompVal', CompVal) );
end

  btype = 60001 + 42;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ CompVal ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_CompareInt32(sim, events, in, Thr) // PARSEDOCU_BLOCK
//
// %PURPOSE: Compeare to Thr (if greater)
// 
// in*, float
// out*, int32 - 0 if (in > CompVal); 1 if (in != CompVal);
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'Thr', Thr) );
end

  btype = 60001 + 47;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ Thr ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim,out] = ld_integratorInt32(sim, events, in) // PARSEDOCU_BLOCK
//
// %PURPOSE: Integrator on Int32
// 
// in*, int32
// out*, int32  out[k] = out[k-1] + in[k]
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
end

  btype = 60001 + 48;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction






















//   ortd_checkpar(sim, list('Signal', 'in', in) );
//   ortd_checkpar(sim, list('String', 'str', str) );
//   ortd_checkpar(sim, list('SingleValue', 'insize', insize) );




// 
//  Vector functions
// 

function [sim, out] = ld_vector_delay(sim, events, in, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: delay - block
//
// in * - input
// out * - output
// vecsize - size of vector in*
// 
// delay the hole vector in by one step
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
end

  btype = 60001 + 65;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ vecsize ], [ ], ...
                   insizes=[vecsize], outsizes=[vecsize], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_vector_delayInt32(sim, events, in, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: delay - block
//
// in * - input (int32)
// out * - output (int32)
// vecsize - size of vector in*
// 
// delay the hole vector in by one step
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
end

  btype = 60001 + 84;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ vecsize ], [ ], ...
                   insizes=[vecsize], outsizes=[vecsize], ...
                   intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_vector_diff(sim, events, in, vecsize) // PARSEDOCU_BLOCK
//    
// %PURPOSE: Vector differentiation with respect to the index
// 
// in *+(vecsize) - vector signal of size "vecsize"
// out *+(vecsize-1) - vector signal of size "vecsize-1"
//
// Equivalent to Scilab 'diff' function
//    

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
end

  btype = 60001 + 50; 
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize], outsizes=[vecsize-1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, index] = ld_vector_findthr(sim, events, in, thr, greater, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: Find the index of the value in a vector that is grater than a given constant
//
// in *+(vecsize) - input
// thr * - threshold signal
// index * - output
// 
// find values greater than threshold "thr" in vector signal "in", when greater > 0
// find values less than threshold "thr" in vector signal "in", when greater =< 0
// 
//
//    
  btype = 60001 + 51; 
  ipar = [vecsize; greater]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize, 1], outsizes=[1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in, thr ) );

  [sim,index] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_vector_abs(sim, events, in, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: Vector abs()
//
// in *+(vecsize) - input
// out *+(vecsize) - output
// 
//    
  btype = 60001 + 52; 
  ipar = [vecsize; 0]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize], outsizes=[vecsize], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_vectorInt32ToFloat(sim, events, in, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: Int32 to Float conversion on a vector 
//
// in *+(vecsize) - input
// out *+(vecsize) - output
// 
//    
  btype = 60001 + 83; 
  ipar = [vecsize; 0]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize], outsizes=[vecsize], ...
                                     intypes=[ ORTD.DATATYPE_INT32 ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_vector_gain(sim, events, in, gain, vecsize) // PARSEDOCU_BLOCK   
// %PURPOSE: Vector gain
//
// in *+(vecsize) - input
// out *+(vecsize) - output
//    
  btype = 60001 + 53; 
  ipar = [vecsize]; rpar = [gain];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize], outsizes=[vecsize], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_vector_extract(sim, events, in, from, window_len, vecsize) // PARSEDOCU_BLOCK
//    
// %PURPOSE: Extract "in" from to from+window_len
// 
//  in *+(vecsize) - vector signal
//  from * - index signal, (indexing starts at 1)
//  out *+(window_len) - output signal
//
//    
  btype = 60001 + 54; 
  ipar = [vecsize; window_len]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize, 1], outsizes=[window_len], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
//   disp('new vextr\n');
//   disp( [vecsize, 1] );


  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in, from ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, index, value] = ld_vector_minmax(sim, events, in, findmax, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: Min / Max of a vector (finds the first appearance of the minimum/maximum)
//
// Function is buggy somehow. Find maximum should work. Minimum perhaps not!
//
// in *+(vecsize)
// findmax greater than 0 means "find the maximum"
// index * - the index starting at 1, where the max / min was found
// value * - min/max value
//    
  btype = 60001 + 55; 
  ipar = [vecsize; findmax]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize], outsizes=[1, 1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,index] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
  [sim,value] = libdyn_new_oport_hint(sim, blk, 1);   // 1th port
endfunction

function [sim, index, FoundSpike, Mean, Sigma, Distance, Val] = ld_vectorFindSpike(sim, events, in, SignificanceFactor, NskipLeft, NskipRight, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: find a spike in a given dataset
//
// Steps performed:
//
// 1) The maximum of abs(in) is determined as well as its position
// 2) The variance (sigma^2) of in is calculated except for the values surounded by
//    the maxmimum. This range is described by NskipLeft and NskipRight
// 3) The maximum is compared to the standard deviation (sigma); also the
//    signal's mean value is compensated herein.
// 4) If the intensity of the maximum is significantly higher than the maximum's
//    intensity, FoundSpike is set to 1 
//
// in *+(vecsize)
// SignificanceFactor - Used for the comparison Distance > SignificanceFactor * sigma, 
// index *(INT32) - the index starting at 1, where the spike was found
// FoundSpike *(INT32) - 1 if a spike has been found. 0 otherwise
//    
  btype = 60001 + 80; 
  ipar = [vecsize, NskipLeft, NskipRight]; rpar = [SignificanceFactor];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize], outsizes=[1, 1, 1, 1, 1, 1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], ...
                                     outtypes=[ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_FLOAT, ...
                                               ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT ] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,index] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
  [sim,FoundSpike] = libdyn_new_oport_hint(sim, blk, 1);   // 1th port
  [sim,Mean] = libdyn_new_oport_hint(sim, blk, 2);   // 1th port
  [sim,Sigma] = libdyn_new_oport_hint(sim, blk, 3);   // 1th port
  [sim,Distance] = libdyn_new_oport_hint(sim, blk, 4);   // 1th port
  [sim,Val] = libdyn_new_oport_hint(sim, blk, 5);   // 1th port
endfunction

function [sim, out, num] = ld_vector_glue(sim, events, in1, fromindex1, toindex1, in2, fromindex2, toindex2, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: Extract parts from two input vectors and glue them together to receive one vector.
//
// Output starting with in1 from fromindex1 until toindex1, continuing with in2 from fromindex2 until toindex2.
// 
// Index counting starts at 1
//
// in1 *+(vecsize)
// in2 *+(vecsize)
// fromindex1 * - first index considered from in1
// toindex1 * - stop cutting from in1 before toindex1. This means the value in1(toindex1) is not included
// fromindex2 * - first index considered from in2
// toindex2 * - stop cutting from in2 before toindex2. This means the value in2(toindex2) is not included
// vecsize - size of each input vector. Vectors need to have equal size!
// out * - as explained above. Size of output is (2*vecsize).
// num * - number of values that have been glued together.
// 
//    
  btype = 60001 + 64;
  outsize = 2*vecsize;
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize, 1, 1, vecsize, 1, 1], outsizes=[outsize, 1], ...
                                     intypes=[ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT], ...
                                     outtypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT]);
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in1,fromindex1,toindex1,in2,fromindex2,toindex2 ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
  [sim,num] = libdyn_new_oport_hint(sim, blk, 1);   // 1th port
endfunction



function [sim, out] = ld_vector_addscalar(sim, events, in, add, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: add "add" to the vector
// 
//  add * - signal
//  in *+(vecsize) - vector signal
//    
  btype = 60001 + 56; 
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize, 1], outsizes=[vecsize], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT  ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in, add ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_vector_add(sim, events, in1, in2, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: add two vectors elementwise
// 
//  in1 *+(vecsize) - vector signal1
//  in2 *+(vecsize) - vector signal2
//    
  btype = 60001 + 66; 
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize, vecsize], outsizes=[vecsize], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT  ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in1, in2 ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_vector_sum(sim, events, in, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: sum over "in"
//
// in *+(vecsize)
// out *
//    
  btype = 60001 + 57;
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize ], outsizes=[1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_vector_addsum(sim, events, in1, in2, vecsize) // FIXME TODO 
// %PURPOSE: multiplicate two vectors and calc the sum of the result ( sum( in1 .* in2) )
//
// in1 *+(vecsize)
// in2 *+(vecsize)
// out *
//    
  btype = 60001 + 58;
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize,vecsize ], outsizes=[1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in1, in2 ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_vector_abssum(sim, events, in, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: sum over element wise abs() of "in"
//
// in *+(vecsize)
// out *
//    
  btype = 60001 + 59;
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize ], outsizes=[1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_vector_sqsum(sim, events, in, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: sum over element wise ()^2 of "in"
//
// in *+(vecsize)
// out *
//    
  btype = 60001 + 60;
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize ], outsizes=[1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_vector_extractandsum(sim, events, in, from, window_len, vecsize) // PARSEDOCU_BLOCK
//    
// %PURPOSE: Extract "in" from "from"-index to "to"-index and sum up (untested) EXPERIMENTAL FOR NOW
// 
//  in *+(vecsize) - vector signal
//  from * - index signal
//  to * - index signal
//
//    
  btype = 60001 + 61;
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize, 1, 1], outsizes=[1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );

  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in, from, to ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_simplecovar(sim, events, in, shape, vecsize) // PARSEDOCU_BLOCK
//    
// %PURPOSE: Cross correlation between a vectorial signal and a given shape, use ld_vecXCorrelation instead
// 
// Note: Use ld_vecXCorrelation instead. This will be removed soon.
//    
// The size of the output vector signal will be vecsize-length(shape) + 1

// FIXME: remove this function

  btype = 60001 + 62;
  ipar = [vecsize, length(shape) ]; rpar = [ shape ];

  if vecsize<length(shape) then
    error("vecsize<length(shape) !");
  end
  
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize  ], outsizes=[ vecsize-length(shape)+1 ], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );

  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_vectorFindShape(sim, events, in, shape, vecsize) // PARSEDOCU_BLOCK
//    
// %PURPOSE: Cross correlation between a vectorial signal and a given shape like ld_vecXCorrelation using ofset compensation
// 
// Prior and after the floating window used to compare with the given shape
// the signal ofset of in at the current window position is estimated
//    
// The size of the output vector signal will be vecsize-length(shape) - 1

//pause;

  btype = 60001 + 79;
  ipar = [vecsize, length(shape) ]; rpar = [ shape ];

  if vecsize<( length(shape) + 2) then
    error("vecsize<length(shape) !");
  end
  
  OutVecSize = vecsize- (length(shape)+2) +1;
  
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize  ], outsizes=[  OutVecSize ], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );

  // printf("Output vector size: %f\n", OutVecSize );

  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_vecXCorrelation(sim, events, in, shape, vecsize) // PARSEDOCU_BLOCK
//    
// %PURPOSE: Cross correlation between a vectorial signal and a given shape
// 
//  in *+(vecsize) - vector signal
//  shape - vector to compare the input with
//  out *+(vecsize-length(shape) + 1) - output
//
// Calculates the cross correlation between "in" and "shape"
//    
// The size of the output vector signal will be vecsize-length(shape) + 1

  btype = 60001 + 62;
  ipar = [vecsize, length(shape) ]; rpar = [ shape ];

  if vecsize<length(shape) then
    error("vecsize<length(shape) !");
  end
  
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize  ], outsizes=[ vecsize-length(shape)+1 ], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );

  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_vector_mute(sim, events, in, from, len, setto, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: mute a vector from and to a spacified index
// 
//  in *+(vecsize) - vector signal
//  from * - signal (index counting starts at )
//  len * - signal (length of the window to mute)
//  setto * - signal
//
//

  btype = 60001 + 63; 
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize, 1, 1, 1, 1], outsizes=[vecsize], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT  ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in, from, len, setto ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



function [sim, out] = ld_vector_NaNtoVal(sim, events, in, Val, vecsize) // PARSEDOCU_BLOCK   
// %PURPOSE: Find all NaN in a vector and set them to Val
//
// in *+(vecsize) - input
// out *+(vecsize) - output
// Val - numeric parameter
//    
  btype = 60001 + 67; 
  ipar = [vecsize]; rpar = [Val];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize], outsizes=[vecsize], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, outlist] = ld_LevelDemux(sim, events, in, NrEvents) // PARSEDOCU_BLOCK
//    
// %PURPOSE: Demux the level of the input signal such that the output corresponding to the input level is set to one
// 
//  in * - vector signal
//  outlist - list() of * with NrEvents elements
//
//  n = round(in) 
//  outlist(m) == 1, for n=m  AND outlist(m) == 0, for m != n
// 
//    

  KeepOutputLevel = 0;

  btype = 60001 + 68;
  ipar = [NrEvents, KeepOutputLevel]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[1], outsizes=[ ones(NrEvents,1) ], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ones(NrEvents,1)*ORTD.DATATYPE_FLOAT] );

  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  outlist = list();
  for i=1:NrEvents 
    [sim,out] = libdyn_new_oport_hint(sim, blk, i-1);   // 0th port
    outlist(i) = out;
  end
  
endfunction


function [sim, out] = ld_TrigSwitch1toN(sim, events, Event, SwitchInputList, InitialState) // PARSEDOCU_BLOCK
//    
// %PURPOSE: Switch N inputs to one output signal based on event pulses
// 
//  Event * - vector signal
//  SwitchInputList list() of * with N elements 
//
//  out * - is set to the input if SwitchInputList(state), whereby "state" is the current state that can be changed 
//          giving inpulses to Event, whose intensity correspond to the state to switch to. Event <0.5 does not change
//          the state.
// 
//    


  
  N = length(SwitchInputList);

  btype = 60001 + 69;
  ipar = [N, InitialState]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[1;ones(N,1)], outsizes=[ 1 ], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT; ones(N,1)*ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );

  // libdyn_conn_equation connects multiple input signals to blocks

  
  
  inlist = list(Event);
  
  for i=1:N
    inlist(i+1) = SwitchInputList(i);
  end
  [sim,blk] = libdyn_conn_equation(sim, blk, inlist );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
  
endfunction

function [sim, out] = ld_vector_concate(sim, events, in1, in2, size1, size2) // PARSEDOCU_BLOCK
  //
  // %PURPOSE: Concatenate two vectors
  //
  // in1, in2 *+ - input vectors
  // out *(size1+size2) - the concatenated vector
  // size1, size2 - respective length for both vectors
  // 
  //

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in1', in1) );
  ortd_checkpar(sim, list('Signal', 'in2', in2) );
  ortd_checkpar(sim, list('SingleValue', 'size1', size1) );
  ortd_checkpar(sim, list('SingleValue', 'size2', size2) );
end

  btype = 60001 + 70; 
  ipar = [ size1, size2 ]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[size1, size2], outsizes=[size1+size2], ...
                       intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in1, in2) );

  // connect each outport
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // ith port
endfunction

function [sim, out] = ld_vector_multscalar(sim, events, in, mult, vecsize) // PARSEDOCU_BLOCK
// %PURPOSE: multiplicate the given vector
// 
//  mult * - signal
//  in *+(vecsize) - vector signal
//    

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('Signal', 'mult', mult) );
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
end

  btype = 60001 + 71; 
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize, 1], outsizes=[vecsize], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT  ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in, mult ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



function [sim, out] = ld_collectValues(sim, events, in, WriteIndex, memorysize, DefaultVal, inVecsize ) // PARSEDOCU_BLOCK
  //
  // %PURPOSE: Store input values in a memory at a given position
  //
  // in * - input vector whose values shall be stored
  // out *(memorysize) - the vector representing the memory
  // WriteIndex * INT32 - the index to write the data to; starts at 1
  // memorysize - storage size
  // DefaultVal - initialize the storage with this value
  // vecsize - size of the input vector
  //

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('Signal', 'WriteIndex', WriteIndex) );
  ortd_checkpar(sim, list('SingleValue', 'memorysize', memorysize) );
  ortd_checkpar(sim, list('SingleValue', 'DefaultVal', DefaultVal) );
  ortd_checkpar(sim, list('SingleValue', 'inVecsize', inVecsize) );
end

  btype = 60001 + 74;
  ipar = [ memorysize, inVecsize ]; rpar = [DefaultVal];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[inVecsize, 1 ], outsizes=[memorysize], ...
                       intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in, WriteIndex) );

  // connect each outport
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // ith port
endfunction


function [sim, out] = ld_HistogramInt32(sim, events, Val, Weight, from, to ) // PARSEDOCU_BLOCK
  //
  // %PURPOSE: Accumulative store input values in a memory at a given position
  //
  // Val * - input 
  // Weight * - int32
  // out *(from-to+1) int32 - the vector representing the histogram
  // 
  // from - min input value in the histogram
  // to - max input value in the histogram
  // 
  //

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'Val', Val) );
  ortd_checkpar(sim, list('Signal', 'Weight', Weight) );
  ortd_checkpar(sim, list('SingleValue', 'from', from) );
  ortd_checkpar(sim, list('SingleValue', 'to', to) );
end

  btype = 60001 + 81;
  ipar = [ from, to ]; rpar = [];
  
//  pause;

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[1, 1 ], outsizes=[to-from+1], ...
                       intypes=[ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(Val, Weight) );

  // connect each outport
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // ith port
endfunction


// [sim, TimerActive, Counter] = ld_Timer(sim, 0, Trigger=AbnormalityDetected, Count=length(par_.CorrModel) )

function [sim, TimerActive, Counter] = ld_Timer(sim, events, Trigger, Count ) // PARSEDOCU_BLOCK
  //
  // %PURPOSE: A timer than can be triggered
  //
  // The timer is active for Count simulation steps
  //
  // Trigger * - (int32) start the timer immediately 
  // TimerActive * - int32 1 if the timer is active, 0 if not
  // Counter * (int32) - The used conter that starts from Count and decreases
  // 
  // 
  //

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'Trigger', Trigger) );
  ortd_checkpar(sim, list('SingleValue', 'Count', Count) );
end

  btype = 60001 + 82;
  ipar = [ Count ]; rpar = [];
  
//  pause;

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[1 ], outsizes=[1,1], ...
                       intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(Trigger) );

  // connect each outport
  [sim, TimerActive] = libdyn_new_oport_hint(sim, blk, 0);   // ith port
  [sim, Counter] = libdyn_new_oport_hint(sim, blk, 1);   // ith port
endfunction



function [sim,out] = ld_add_ofsInt32(sim, events, in, ofs) // PARSEDOCU_BLOCK
//
// %PURPOSE: add ofs(in)
// 
// input and return value are of type ORTD.DATATYPE_INT32
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'ofs', ofs) );
end

  btype = 60001 + 75;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ ofs ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_gainInt32(sim, events, in, fac) // PARSEDOCU_BLOCK
//
// %PURPOSE: integer multiplication of (in)
// 
// input and return value are of type ORTD.DATATYPE_INT32
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'fac', fac) );
end

  btype = 60001 + 76;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ fac ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



function [sim,out] = ld_VarVec_ztf(sim, events, in, Nvalues, H, FilterMode, vecsize) // PARSEDOCU_BLOCK
//
// %PURPOSE: discrete-time transfer function applied to vector data
// 
// FilterMode == 1 apply filter from left to right
// FilterMode == 2 apply filter from right to left
// FilterMode == 3 apply filter from left to right, flip result, filter from right to left (zero phase-shift filter)
// 
// H is a transfer function in z-domain represented by a scilab rational
//
// in (float, vecsize) - vector input
// out (float, vecsize) - vector output
// Nvalues (int32) - the number of samples the filter is applied to (starting from the left side of the vector)
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('Signal', 'Nvalues', Nvalues) );
  
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
  ortd_checkpar(sim, list('SingleValue', 'FilterMode', FilterMode) );
end

//  bip = [ degree(H.num); degree(H.den) ];
//  brp = [ coeff(H.num)'; coeff(H.den)' ];

  btype = 60001 + 85;
  [sim,blk] = libdyn_new_block(sim, events, btype, [ degree(H.num), degree(H.den) , vecsize, FilterMode ], [ coeff(H.num)'; coeff(H.den)' ], ...
                   insizes=[ vecsize, 1 ], outsizes=[ vecsize ], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in, Nvalues) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim,out] = ld_VarVec_add(sim, events, inlist, Nvalues, weight, vecsize) // PARSEDOCU_BLOCK
//
// %PURPOSE: Linear combination of two vectors
// 
// 
//
// inlist (float, vecsize) - list() of vector input
// out (float, vecsize) - vector output
// Nvalues (int32) - the number of samples the filter is applied to (starting from the left side of the vector)
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'inlist(1)', inlist(1) ) );
  ortd_checkpar(sim, list('Signal', 'inlist(2)', inlist(2) ) );
  ortd_checkpar(sim, list('Signal', 'Nvalues', Nvalues) );
  
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
  ortd_checkpar(sim, list('SingleValue', 'weight(1)', weight(1) ) );
  ortd_checkpar(sim, list('SingleValue', 'weight(2)', weight(2) ) );
end

//  bip = [ degree(H.num); degree(H.den) ];
//  brp = [ coeff(H.num)'; coeff(H.den)' ];

  btype = 60001 + 87;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  vecsize ], [ weight(1), weight(2) ], ...
                   insizes=[ vecsize, vecsize, 1 ], outsizes=[ vecsize ], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(inlist(1), inlist(2), Nvalues) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, Mean, Sigma] = ld_VarVec_Variance(sim, events, in, Nvalues, vecsize) // PARSEDOCU_BLOCK
//
// %PURPOSE: Calc variance of vector elements
// 
// 
//
// in (float, vecsize) - vector input
// out (float, vecsize) - vector output
// Nvalues (int32) - the number of samples the filter is applied to (starting from the left side of the vector)
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in ) );
  ortd_checkpar(sim, list('Signal', 'Nvalues', Nvalues) );
  
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
end



  btype = 60001 + 88;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  vecsize ], [ ], ...
                   insizes=[ vecsize, 1 ], outsizes=[ 1,1 ], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in, Nvalues) );
  [sim,Mean] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
  [sim,Sigma] = libdyn_new_oport_hint(sim, blk, 1);   // 0th port
endfunction


function [sim, out] = ld_VarVec_AbsSumNorm(sim, events, in, Nvalues, vecsize) // PARSEDOCU_BLOCK
//
// %PURPOSE: Calc sum of the absolute value of vector elements
// 
// 
//
// in (float, vecsize) - vector input
// out (float, vecsize) - vector output
// Nvalues (int32) - the number of samples the filter is applied to (starting from the left side of the vector)
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in ) );
  ortd_checkpar(sim, list('Signal', 'Nvalues', Nvalues) );
  
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
end



  btype = 60001 + 90;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  vecsize ], [ ], ...
                   insizes=[ vecsize, 1 ], outsizes=[ 1 ], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in, Nvalues) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



function [sim, Min, Max] = ld_VarVec_MinMax(sim, events, in, Nvalues, vecsize) // PARSEDOCU_BLOCK
//
// %PURPOSE: Calc Min and Max of vector elements
// 
// 
//
// in (float, vecsize) - vector input
// out (float, vecsize) - vector output
// Nvalues (int32) - the number of samples the filter is applied to (starting from the left side of the vector)
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in ) );
  ortd_checkpar(sim, list('Signal', 'Nvalues', Nvalues) );
  
  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
end



  btype = 60001 + 91;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  vecsize ], [ ], ...
                   insizes=[ vecsize, 1 ], outsizes=[ 1,1 ], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in, Nvalues) );
  [sim,Min] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
  [sim,Max] = libdyn_new_oport_hint(sim, blk, 1);   // 0th port
endfunction

//function [sim, out] = ld_VarVec_Abs(sim, events, in, Nvalues, vecsize) // PARSEDOCU_BLOCK
////
//// %PURPOSE: Calc the absolute values of vector elements
//// 
//// 
////
//// in (float, vecsize) - vector input
//// out (float, vecsize) - vector output
//// Nvalues (int32) - the number of samples the filter is applied to (starting from the left side of the vector)
////
//
//if ORTD.FASTCOMPILE==%f then
//  ortd_checkpar(sim, list('Signal', 'in', in ) );
//  ortd_checkpar(sim, list('Signal', 'Nvalues', Nvalues) );
//  
//  ortd_checkpar(sim, list('SingleValue', 'vecsize', vecsize) );
//end
//
//
//
//  btype = 60001 + 92;
//  [sim,blk] = libdyn_new_block(sim, events, btype, [  vecsize ], [ ], ...
//                   insizes=[ vecsize, 1 ], outsizes=[ vecsize ], ...
//                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_INT32], outtypes=[ORTD.DATATYPE_FLOAT]  );
//
//  [sim,blk] = libdyn_conn_equation(sim, blk, list(in, Nvalues) );
//  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
//endfunction
//



function [sim, out, Nvalues] = ld_vector_VarExtract(sim, events, in, from, to, vecsize) // PARSEDOCU_BLOCK
//    
// %PURPOSE: Extract vector elements from a window variable in size 
// 
//  in *+(vecsize) - vector signal
//  from, to (INT32) - cut parameters, indices start at 1
//  out *+(window_len) - output signal
//  Nvalues (INT32) - number of the elements cut
//    
  btype = 60001 + 86; 
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize, 1, 1], outsizes=[vecsize, 1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32 ], ....
                                     outtypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_INT32] );

  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in, from, to ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
  [sim,Nvalues] = libdyn_new_oport_hint(sim, blk, 1);   // 1th port
endfunction






function [sim, out] = ld_switch2to1Int32(sim, events, cntrl, in1, in2) // PARSEDOCU_BLOCK
//
// %PURPOSE: A 2 to 1 switching Block
//
// cntr (INT32) - control input
// in1 (INT32)
// in2 (INT32)
// out (INT32) - output
//
// if cntrl > (greather than) : out = in1
//   else                     : out = in2
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'cntrl', cntrl) );
  ortd_checkpar(sim, list('Signal', 'in1', in1) );
  ortd_checkpar(sim, list('Signal', 'in2', in2) );
end


  btype = 60001 + 89;
  [sim,blk] = libdyn_new_block(sim, events, btype, [], [], ...
                   insizes=[1, 1, 1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32, ORTD.DATATYPE_INT32], ...
                   outtypes=[ORTD.DATATYPE_INT32]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(cntrl, in1, in2) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_max(sim, events, inlist) // PARSEDOCU_BLOCK
// %PURPOSE: logic and - block
//
// in *LIST - list() of inputs (for now the exactly two inputs are possible)
// out * - output
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('SignalList', 'inlist', inlist) );
end

  Nin=length(inlist);

  if (Nin ~= 2) then
    error("invalid number of inputs");
  end

  insizes=ones(1, Nin);
  intypes=ones(1, Nin) * ORTD.DATATYPE_FLOAT;

  btype = 60001 + 92;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[  ], rpar=[   ], ...
                   insizes, outsizes=[1], ...
                   intypes, outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list( inlist(1), inlist(2) ) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


//
//// FIXME no port size checking
//function [sim,bid] = libdyn_new_blk_zTF(sim, events, H)
//  btype = 30;
//  bip = [ degree(H.num); degree(H.den) ];
//  brp = [ coeff(H.num)'; coeff(H.den)' ];
//
//  [sim,bid] = libdyn_new_block(sim, events, btype, [bip], [brp], ...
//                   insizes=[1], outsizes=[1], ...
//                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]);
//  
////  [sim,bid] = libdyn_new_blockid(sim);
////  id = bid; // id for this parameter set
////  
////  
////  sim.parlist = new_irparam_elemet(sim.parlist, id, IRPAR_LIBDYN_BLOCK, [btype; bid; bip], [brp]);
//endfunction
//function [sim,y] = ld_ztf(sim, events, inp_list, H) // PARSEDOCU_BLOCK
////
//// %PURPOSE: Time discrete transfer function
//// H is give as a Scilab rational
////
//
//if ORTD.FASTCOMPILE==%f then
//  ortd_checkpar(sim, list('Signal', 'inp_list', inp_list) );
//end
//
//  [inp] = libdyn_extrakt_obj( inp_list ); // compatibility
//
//    [sim,tf] = libdyn_new_blk_zTF(sim, events, H);
//    
//    
//    
//    [sim,y] = libdyn_conn_equation(sim, tf, list(inp));
//    [sim,y] = libdyn_new_oport_hint(sim, y, 0);    
//endfunction


// 
// 
// 
// 
// Blocks that use the new Cpp Interfac
// 
// 
// 
// 



function [sim, out] = ld_RTCrossCorr(sim, events, u, shapeSig, len) // PARSEDOCU_BLOCK
// 
// Online Cross Correlation
//
// u * - input signal
// shapeSig * - input shape
// out * - output signal
// len - length of input shape
// 
// 
// Note: The implementation is not optimal. Only the raw sum formula is evaluated.
// 

// introduce some parameters that are refered to by id's

// Set-up the block parameters and I/O ports
  Uipar = [ ];
  Urpar = [ ];
  btype = 60001 + 300;

  insizes=[len,1]; // Input port sizes
  outsizes=[1]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT]; // datatype for each input port
  outtypes=[ORTD.DATATYPE_FLOAT]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list(shapeSig, u) ); // connect in1 to port 0 and in2 to port 1

  // connect the ouputs
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction




function [sim, out] = ld_ReadAsciiFile(sim, events, fname, veclen) // PARSEDOCU_BLOCK
// 
// Read data from an ascii-file
//
// fname - file name (string)
// veclen - Size of the vector to read during each time-step
// out *(veclen) - Output signal as read from the file
// 
// The data contained in the file must be ascii data divided into rows and columns
// as it may be read on Scilab using the command "fscanfMat".
// The number of columns must be veclen. 
// For each time-step the respectively next row of data is read from the file and
// copied to the block's output vectorial signal "out"
// 
// The file must be available only at runtime.
// 
// 


   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, ascii(fname), 12); // id = 12; A string parameter

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 60001 + 301; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[]; // Input port sizes
  outsizes=[veclen]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[]; // datatype for each input port
  outtypes=[ORTD.DATATYPE_FLOAT]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed);
  
  // connect the inputs
//  [sim,blk] = libdyn_conn_equation(sim, blk, list(in1, in2) ); // connect in1 to port 0 and in2 to port 1

  // connect the ouputs
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_ArrayInt32(sim, events, array, in) // PARSEDOCU_BLOCK
// 
// Lookup a value inside an array - block
//
// in * - input
// out * - output
// 
// out = array[in]
// 

if ORTD.FASTCOMPILE==%f then
   ortd_checkpar(sim, list('Signal', 'in', in) );
  //FIXME check array
end

// introduce some parameters that are refered to by id's

   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, array, 10); // id = 10

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 60001 + 303; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[1]; // Input port sizes
  outsizes=[1]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[ORTD.DATATYPE_INT32]; // datatype for each input port
  outtypes=[ORTD.DATATYPE_INT32]; // datatype for each output port

  blocktype = 2; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list(in) ); // connect in1 to port 0 and in2 to port 1

  // connect the ouputs
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



function [sim, out] = ld_const_bin(sim, events, BinConst) // PARSEDOCU_BLOCK
// 
// %PURPOSE: a constant vector of ORTD.DATATYPE_BINARY
// 
// out *+ - the vector of binary
// 


  
  //ortd_checkpar(sim, list('', 'BinConst', BinConst) );

  // pack all parameters into a structure "parlist"
  parlist = new_irparam_set();
  insize = length(BinConst);
  parlist = new_irparam_elemet_ivec(parlist, insize, 10); // id = 10
  parlist = new_irparam_elemet_ivec(parlist, BinConst, 11); // id = 11
  
  p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

  // Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 60001 + 304; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[];
  outsizes=[ insize ]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[];
  outtypes=[ ORTD.DATATYPE_BINARY  ]; // datatype for each output port

  blocktype = 2; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed);
  

  // connect the ouputs
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim] = ld_ORTDIO_Put(sim, events, in, len, datatype, header) // PARSEDOCU_BLOCK
// 
// Put data to ORTD_IO
//
// len - Size of the vector to be send
// in *(len) - Input signal.
// datatype - Datatype of signal "in"
// header - A string that is prepended to each binary message
// 


   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, ascii(header), 12); // id = 12; A string parameter

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 60001 + 305; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[len]; // Input port sizes
  outsizes=[]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[datatype]; // datatype for each input port
  outtypes=[]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed);
  
  // connect the inputs
  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) ); // 

  // connect the ouputs
 //[sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim] = ld_SyncFilewrite(sim, events, in, len, datatype, fname, trigger, par) // PARSEDOCU_BLOCK
// 
// Synchronously write ascii data to a file
//
// Data is stored directly during the flag for updating states and may hence disturbe realtime operation
// of a surrounding realtime loop. Only datatype float is currently supported.
//
// len - Size of the vector to be send
// in *(len) - Input signal.
// datatype - Datatype of signal "in"
// fname - A string that is prepended to each binary message
// par - optional parameters (none at the moment. Put par=struct() )
// 


   // pack all parameters into a structure "parlist"
   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, ascii(fname), 12); // id = 12; A string parameter

   p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively

// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 60001 + 306; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[len,1]; // Input port sizes
  outsizes=[]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[datatype, ORTD.DATATYPE_INT32]; // datatype for each input port
  outtypes=[]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed);
  
  // connect the inputs
  [sim,blk] = libdyn_conn_equation(sim, blk, list(in, trigger) ); // 

  // connect the ouputs
 //[sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, EstMean, EstSigma] = ld_WindowedMean(sim, events, in, weight, WindowLen) // PARSEDOCU_BLOCK
// 
// ld_WindowedMean - Calculte the average of the input singal for a floating window
//
// in * - input signal
// weight * - weight for the current sample
// EstMean * - the calculated average
// EstSigma * - the calculated standard deviation
// WindowLen - length of floating window
// 
// If weight is less than 0 the filter update is prevented. E.g. if in contains an invalid sample that
// shall not be counted in the calculation of the Mean and Variance
// 


// introduce some parameters that are refered to by id's

// Set-up the block parameters and I/O ports
  Uipar = [ WindowLen ];
  Urpar = [ ];
  btype = 60001 + 307;

  insizes=[1,1]; // Input port sizes
  outsizes=[1,1]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT]; // datatype for each input port
  outtypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list(in, weight) ); // connect in1 to port 0 and in2 to port 1

  // connect the ouputs
 [sim,EstMean] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
 [sim,EstSigma] = libdyn_new_oport_hint(sim, blk, 1);   // 0th port
 
endfunction








// 
// 
// Special blocks
// 
// 


// obsolete
function [sim,bid] = libdyn_new_interface(sim, events, len)
  btype = 4000;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [len], []);
endfunction


function [sim, out] = ld_interface(sim, events, in, vecsize) // PARSEDOCU_BLOCK   
// Interfacing block
//
// in *+(vecsize) - input
// out *+(vecsize) - output
//    
  btype = 60001 + 1000; 
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize], outsizes=[vecsize], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction





//
// Macros
//
//


function [sim,y] = ld_add_ofs(sim, events, u, ofs) // PARSEDOCU_BLOCK
//
// %PURPOSE: Add a constant "ofs" to the signal u; y = u + const(ofs)
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'u', u) );
  ortd_checkpar(sim, list('SingleValue', 'ofs', ofs) );
end

  [sim,ofs_const] = libdyn_new_blk_const(sim, events, ofs);
  
  [sim,y] = ld_sum(sim, events, list(u,0, ofs_const,0), 1,1);
endfunction


function [sim, y] = ld_mute( sim, ev, u, cntrl, mutewhengreaterzero ) // PARSEDOCU_BLOCK
//    
//    %PURPOSE: Mute a signal based on cntrl-signal
//
//    ev - event
//    u * - input
//    y * - output
//    mutewhengreaterzero - boolean parameter (%T, %F)
//    
//    if mutewhengreaterzero == %T then
//
//      y = 0 for cntrl > 0
//      y = u for cntrl < 0
//
//    else
//
//      y = 0 for cntrl < 0
//      y = u for cntrl > 0
//
//
//
    
    [sim, zero] = ld_const(sim, ev, 0);
    
    if (mutewhengreaterzero == %T) then
      [sim,y] = ld_switch2to1(sim, ev, cntrl, zero, u);
    else
      [sim,y] = ld_switch2to1(sim, ev, cntrl, u, zero);
    end
endfunction



function [sim, y] = ld_limited_integrator(sim, ev, u, min__, max__, Ta) // PARSEDOCU_BLOCK
// %PURPOSE: Implements a time discrete integrator with saturation of the output between min__ and max__
// 
// u * - input
// y * - output
// 
// y(k+1) = sat( y(k) + Ta*u , min__, max__ )
    [sim, u__] = ld_gain(sim, ev, u, Ta);
    
    [sim,z_fb] = libdyn_new_feedback(sim);
    
  [sim, sum_] = ld_sum(sim, ev, list(u__, z_fb), 1, 1);
  [sim, tmp] = ld_ztf(sim, ev, sum_, 1/z);
  [sim, y] = ld_sat(sim, ev, tmp, min__, max__);
    
    [sim] = libdyn_close_loop(sim, y, z_fb);    
endfunction

function [sim, y] = ld_limited_integrator2(sim, ev, u, min__, max__, Ta) // PARSEDOCU_BLOCK
// %PURPOSE: Implements a time discrete integrator with saturation of the output between min__ and max__
// compared to ld_limited_integrator there is no delay: Ta z / (z-1)
//
// u * - input
// y * - output
// 
// y(k+1) = sat( y(k) + Ta*u , min__, max__ )


    [sim, u__] = ld_gain(sim, ev, u, Ta);
    
    [sim,z_fb] = libdyn_new_feedback(sim);
    
  [sim, sum_] = ld_sum(sim, ev, list(u__, z_fb), 1, 1);
  [sim, y] = ld_sat(sim, ev, sum_, min__, max__);


  [sim, y_] = ld_ztf(sim, ev, y, 1/z);
    
    [sim] = libdyn_close_loop(sim, y_, z_fb);    

endfunction

function [sim, y] = ld_limited_integrator3(sim, ev, u, min__, max__, Ta) // PARSEDOCU_BLOCK
// %PURPOSE: Implements a time discrete integrator (trapeziodal rule) with saturation of the output between min__ and max__
//
// u * - input
// y * - output
// 
// y(k+1) = sat(  Ta/2 ( u(k)+u(k+1) ) + y(k), min__, max__ )

    ukp1 = u;
    [sim, uk] = ld_ztf(sim, ev, ukp1, 1/z);
    [sim, u__] = ld_add(sim, ev, list(uk, ukp1), [Ta/2, Ta/2] );     
//     [sim, u__] = ld_gain(sim, ev, tmp, Ta/2);
    
    [sim,z_fb] = libdyn_new_feedback(sim);
    
  [sim, sum_] = ld_sum(sim, ev, list(u__, z_fb), 1, 1);
  [sim, y] = ld_sat(sim, ev, sum_, min__, max__);

  [sim, y_] = ld_ztf(sim, ev, y, 1/z);
    
    [sim] = libdyn_close_loop(sim, y_, z_fb);    
endfunction

function [sim, y] = ld_limited_integrator4(sim, ev, u, min__, max__, Ta) // PARSEDOCU_BLOCK
// %PURPOSE: Implements a time discrete integrator with saturation of the output between min__ and max__
// compared to ld_limited_integrator there is no delay: Ta z / (z-1)
//
// u * - input
// y * - output
// min__ * - variable saturation minimum
// 
// y(k+1) = sat( y(k) + Ta*u , min__, max__ )


    [sim, u__] = ld_gain(sim, ev, u, Ta);
    
    [sim,z_fb] = libdyn_new_feedback(sim);
    
  [sim, sum_] = ld_sum(sim, ev, list(u__, z_fb), 1, 1);
  [sim, y_sat] = ld_sat(sim, ev, sum_, 0, max__);
  
  [sim, y_missing] = ld_add(sim, ev, list(min__, y_sat), [1,-1]);
  [sim, is_missing] = ld_compare_01(sim, ev, in=y_missing,  thr=0);
  [sim, y_add] = ld_mult(sim, ev, inp_list=list(y_missing, is_missing), muldiv1_list=[0, 0]);
  [sim, y] = ld_add(sim, ev, list(y_sat, y_add), [1,1]);

  [sim, y_] = ld_ztf(sim, ev, y, 1/z);
    
    [sim] = libdyn_close_loop(sim, y_, z_fb);    

endfunction

function [sim, u] = ld_lin_awup_controller(sim, ev, r, y, Ta, tfR, min__, max__) // PARSEDOCU_BLOCK
// %PURPOSE: linear controller with anti reset windup implemented by bounding the integral state:
// e = r-y
// u = ld_limited_integrator( e, min__, max__ ) + tfR*e
    [sim, e] = ld_sum(sim, ev, list(r, y), 1, -1);
    
    [sim,u1] = ld_limited_integrator(sim, ev, e, min__, max__, Ta);
    [sim,u2] = ld_ztf(sim, ev, e, tfR);
    
    [sim,u] = ld_sum(sim, ev, list(u1,u2), 1,1);
endfunction


function [sim] = ld_print_angle(sim, ev, alpha, text) // PARSEDOCU_BLOCK
// %PURPOSE: Convert an angle in rad to degree and print to console
// 
// alpha * - angle signal
// text - string
// 
    [sim, alpha_deg] = ld_gain(sim, ev, alpha, 1/%pi*180);
    [sim] = ld_printf(sim, ev, alpha_deg, text, 1);
endfunction

function [sim, pwm] = ld_pwm(sim, ev, plen, u) // PARSEDOCU_BLOCK
// 
// %PURPOSE: PWM generator
// 
// plen - period length
// u * - modulation signal; Values are between 0 and 1.
// pwm * - pwm output
//

    [sim,u] = ld_gain(sim, ev, u, plen);
    
    [sim,one] = ld_const(sim, ev, 1);
    
    [sim,modcount] = ld_modcounter(sim, ev, in=one, initial_count=0, mod=plen);
    
    [sim, test] = ld_add(sim, ev, list(modcount, u), [-1,1] );
    [sim,pwm] = ld_compare_01(sim, ev, test,  thr=0);
endfunction

function [sim, outvec, Nvecplay] = ld_vector_play(sim, ev, A, special) // PARSEDOCU_BLOCK
// 
// %PURPOSE: Play a vectorial signal
// 
// A - matrix containing the vectors to play
// outvec*+(length(A(:,1))) - 
// Nvecplay = length of output signal vector
//
// outputs A(:,i), where i is increasing within each time step
// special = [ "repeate" ]
//

  [Nvecplay,Nsamples] = size(A); // m is the number of samples in time

  data = A(:); Ndata = length(data);

  // create a new vector
  [sim,vector] = ld_constvec(sim, ev, data );
  
  // vector extract test
  
  if special == "repeate" then
    [sim,one] = ld_const(sim, ev, 1);
    [sim, index] = ld_modcounter(sim, ev, in=one, initial_count=0, mod=Nsamples);
  else
    error("wrong special string. Should be one of ""repeate"", ...");
  end

//  [sim] = ld_printf(sim, ev, index, "index = ", 1);

  [sim, start_at] = ld_gain(sim, ev, index, Nvecplay);
  [sim, start_at] = ld_add_ofs(sim, ev, start_at, 1);

  [sim,outvec] = ld_vector_extract(sim, ev, in=vector, from=start_at, window_len=Nvecplay, vecsize=Ndata );
endfunction


function [sim,y] = ld_alternate( sim, ev, start_with_zero ) // PARSEDOCU_BLOCK
//
// %PURPOSE: generate an alternating sequence     
//
// y * - output
//
//
// [0, 1, 0, 1, 0, ... ], if start_with_zero == %T
// [1, 0, 1, 0, 1, ... ], if start_with_zero == %F
//
  z = poly(0,'z');    

  [sim,one] = ld_const(sim, ev, 1);

  [sim, fb] = libdyn_new_feedback();
  
  [sim, su ] = ld_add(sim, ev, list(fb, one), [-1,1] );
  [sim, del] = ld_ztf(sim, ev, su, 1/z); // a delay of one sample
  
  [sim] = libdyn_close_loop(sim, del, fb);
  
  y = del; 
  
  if (start_with_zero == %F) then
    [sim, y] = ld_not(sim, ev, y);
  end
    
endfunction

function [sim, out] = ld_detect_step_event(sim, ev, in, eps) // PARSEDOCU_BLOCK
    //
    // %PURPOSE: step detection block
    //
    // Detect jumps in the signal "in".
    // Everytime a jump occurs "out" is an impulse with the intensity of the
    // value after the jump i.e. if a signal steps from 1 to 2 there
    // will be an impulse out = 2
    // if no steps occur, out is zero
    //
    
    z = poly(0, 'z');
    
    [sim, i1] = ld_ztf(sim, ev, in, (z-1)/z ); // diff
    [sim, i2] = ld_abs(sim, ev, i1);
    
    [sim,event] = ld_compare_01(sim, ev, in=i2, thr=eps);
    [sim, out] = ld_mult(sim, ev, inp_list=list(event, in), muldiv1_list=[0, 0]);
    
endfunction

function [sim, out] = ld_detect_step_event2(sim, ev, in, eps) // PARSEDOCU_BLOCK
    //
    // %PURPOSE: step detection block
    //
    // Detect jumps in the signal "in".
    // Everytime a jump occurs "out" is an impulse with an intensity of 1,
    // else it is zero
    //
    
    z = poly(0, 'z');
    
    [sim, i1] = ld_ztf(sim, ev, in, (z-1)/z ); // diff
    [sim, i2] = ld_abs(sim, ev, i1);
    
    [sim,event] = ld_compare_01(sim, ev, in=i2, thr=eps);
    out = event;
    
endfunction

function [sim, reached] = ld_reference_reached(sim, ev, r, y, N, eps) // PARSEDOCU_BLOCK
    //
    // %PURPOSE: check wheter a reference value is reached 
    //
    // r * - Reference Signal (shall be constant)
    // y * - Signal to compare to the reference
    // N - After the condition of reaching the reference is true
    //     N time steps will be wait until setting reached to one. 
    // eps - the half tolerance band width
    //

  [sim, e] = ld_add(sim, ev, list(r,y), [1,-1] );
  //[sim, i1] = ld_ztf(sim, ev, e, 1/(3+1) * (1 + z^(-1) + z^(-2) + z^(-3) ) );

  i1 = e;

  [sim, i3] = ld_abs(sim, ev, i1);
  [sim, reached] = ld_belowEpsForNSteps(sim, ev, in=i3,  thr=eps, N);

endfunction

function [sim, reached] = ld_greaterEpsForNSteps(sim, ev, in, thr, N) // PARSEDOCU_BLOCK
    //
    // %PURPOSE: return true, when input is greater a constant for more than N sampling steps
    //
    // in * - input signal
    // thr - threshold constant
    // N - integer
    //
    // If in is greater than thr for more than N time steps, reached = 1;
    // Otherwise reached is set to 0.
    
  [sim, i4] = ld_compare_01(sim, ev, in,  thr);
  [sim, i5] = ld_not(sim, ev, in=i4);
  
  [sim, resetto] = ld_const(sim, ev, 0);
  [sim, count] = ld_counter(sim, ev, count=i4, reset=i5, resetto, initial=0);

  [sim, reached] = ld_compare_01(sim, ev, in=count,  thr=N);
    
endfunction

function [sim, reached] = ld_belowEpsForNSteps(sim, ev, in, thr, N) // PARSEDOCU_BLOCK
    //
    // %PURPOSE: return true, when input is below a constant for more than N sampling steps
    //
    // in * - input signal
    // thr - threshold constant
    // N - integer
    //
    // If in is below thr for more than N time steps, reached = 1;
    // Otherwise reached is set to 0.
    
  [sim, i4] = ld_compare_01(sim, ev, in,  thr);
  [sim, i5] = ld_not(sim, ev, in=i4);
  
  [sim, resetto] = ld_const(sim, ev, 0);
  [sim, count] = ld_counter(sim, ev, count=i5, reset=i4, resetto, initial=0);

  [sim, reached] = ld_compare_01(sim, ev, in=count,  thr=N);
    
endfunction


function [sim, reached, count, countup, reset] = ld_belowEpsForNStepsAdvanced(sim, ev, in, thr, N) // PARSEDOCU_BLOCK
    //
    // %PURPOSE: return true, when input is below a constant for more than N sampling steps
    //
    // in * - input signal
    // thr - threshold constant
    // N - integer
    //
    // If in is below thr for N time steps, reached = 1;
    // Otherwise reached is set to 0.
    
  [sim, reset] = ld_compare_01(sim, ev, in,  thr);
  [sim, countup] = ld_not(sim, ev, in=reset);
  
  [sim, resetto] = ld_const(sim, ev, 0);
  [sim, count] = ld_counter(sim, ev, countup, reset, resetto, initial=0);

  [sim, reached] = ld_compare_01(sim, ev, in=count,  thr=N-1);
    
endfunction

function [sim] = ld_file_save_machine(sim, ev, in, cntrl, intype, insize, fname) // PARSEDOCU_BLOCK
//
// %PURPOSE: Start and stop saving of data to files
// 
//   in *+(size) - Data to write
//   cntrl * - if cntrl steps to 2 then saving is started; if it steps to 1 saving is stopped
//   intype - ORTD input type of data
//   size - amount of elements in the vector in
//   fname - string: Filename for saving
// 
// Note: The implementation of this function is a superblock using state machines
//       and the ld_savefile block. If you're interested take the source as an example.
// 
// 

    function [sim, outlist, active_state, x_global_kp1, userdata] = state_mainfn(sim, inlist, x_global, state, statename, userdata)
      // This function is called multiple times -- once for each state.
      // At runtime, these are three different nested simulations. Switching
      // between them represents state changing, thus each simulation 
      // represents a certain state.
      
//       printf("defining savemachine state %s (#%d) ... userdata(1)=%s\n", statename, state, userdata(1) );
      
      // define names for the first event in the simulation
      ev = 0; events = ev;

      // 
      dataToSave = inlist(1);
      switch = inlist(2);  [sim, switch] = ld_gain(sim, ev, switch, 1);


      // demultiplex x_global
      [sim, x_global] = ld_demux(sim, events, vecsize=1, invec=x_global);


      // The signals "active_state" is used to indicate state switching: A value > 0 means the 
      // the state enumed by "active_state" shall be activated in the next time step.
      // A value less or equal to zero causes the statemachine to stay in its currently active
      // state

      select state
  case 1 // state 1
    active_state = switch;
    [sim] = ld_printf(sim, ev, in=dataToSave, str="Pauseing Save", insize=DataLen);

  case 2 // state 2
    [sim] = ld_savefile(sim, ev, fname, source=dataToSave, vlen=DataLen);
    [sim] = ld_printf(sim, ev, in=dataToSave, str="Saveing", insize=DataLen);

    active_state = switch;
      end

      // multiplex the new global states
      [sim, x_global_kp1] = ld_mux(sim, ev, vecsize=1, inlist=x_global);
      
      // the user defined output signals of this nested simulation
      outlist = list();
  endfunction

  DataLen = insize;


//   [sim] = ld_printf(sim, ev, cntrl, "cntrl", 1);

  [sim, cntrl] = ld_detect_step_event(sim, ev, in=cntrl, eps=0.2);

//   [sim] = ld_printf(sim, ev, cntrl, "Dcntrl", 1);


  // set-up two states represented by two nested simulations
  [sim, outlist, x_global, active_state,userdata] = ld_statemachine(sim, ev=0, ...
      inlist=list(in, cntrl), ..
      insizes=[insize,1], outsizes=[], ... 
      intypes=[intype ,ORTD.DATATYPE_FLOAT  ], outtypes=[], ...
      nested_fn=state_mainfn, Nstates=2, state_names_list=list("pause", "save"), ...
      inittial_state=1, x0_global=[1], userdata=list("hallo")  );


endfunction







// 
// Blocks, which C functions have not been move to the basic module yet, but the interfacing function
// 



// compare block. If input > thr: 
// optional_cmp_mode342 == 0: output = 1; else -1
// optional_cmp_mode342 == 1: output = 1; else 0
function [sim,bid] = libdyn_new_compare(sim, events, thr, optional_cmp_mode342)    
  if (exists('optional_cmp_mode342') ~= 1) then
    optional_cmp_mode342 = 0;
  end

  btype = 140;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [optional_cmp_mode342], [thr]);
endfunction
function [sim,y] = ld_compare(sim, events, in,  thr) // PARSEDOCU_BLOCK
//
// %PURPOSE: compare block. 
//   thr - constant
//   in * - signal
//   y *
// If in > thr: y = 1; else y = -1
// 
// Please note: returns -1 for in == 0
//

    [sim,blk] = libdyn_new_compare(sim, events, thr);
    [sim,blk] = libdyn_conn_equation(sim, blk, list(in,0));
    [sim,y] = libdyn_new_oport_hint(sim, blk, 0);    
endfunction
function [sim,y] = ld_compare_01(sim, events, in,  thr) // PARSEDOCU_BLOCK
//
// %PURPOSE: compare block. 
//   thr - constant
//   in - signal
//   y *
// If in > thr: y = 1; else y = 0
//

    [sim,blk] = libdyn_new_compare(sim, events, thr, 1); // mode = 1
    [sim,blk] = libdyn_conn_equation(sim, blk, list(in,0));
    [sim,y] = libdyn_new_oport_hint(sim, blk, 0);    
endfunction


















//
// Blocks
//


// FIXME: komische Funktion (noch gebraucht?)
function [sim,bid] = libdyn_new_blk_sum_pn(sim)
  btype = 10;
  [sim,oid] = libdyn_new_objectid(sim);
  id = oid; // id for this parameter set
  
  sim.parlist = new_irparam_elemet(sim.parlist, id, IRPAR_LIBDYN_BLOCK, [btype; oid], []);
endfunction

// FIXME: komische Funktion (noch gebraucht?)
function [sim,bid] = libdyn_new_blk_gen(sim, events, symbol_name, ipar, rpar)
  btype = 5000;
  str = str2code(symbol_name);
  
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [length(str); str; ipar], [rpar]);
endfunction











// 
// 

// // function [sim,bid] = libdyn_new_blk_fngen(sim, events, shape_)
// //   btype = 80;
// //   [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [shape_], []);
// // endfunction
// function [sim,out] = ld_fngen(sim, events, shape_) // PARSEDOCU_BLOCK
// //
// // %PURPOSE: function generator
// // 
// // shape_ - the shape of the output signal: =0 : ???
// // out * - output
// // 
// //
// 
//   ortd_checkpar(sim, list('SingleValue', 'shape_', shape_) );
// 
//   btype = 80;
//   [sim,bid] = libdyn_new_block(sim, events, btype, [shape_], [], ...
//                    insizes=[], outsizes=[1], ...
//                    intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]  );
// 
//   [sim,out] = libdyn_conn_equation(sim, bid, list());
//   [sim,out] = libdyn_new_oport_hint(sim, out, 0);
// endfunction






 
// serial to parallel
function [sim,bid] = libdyn_new_blk_s2p(sim, events, len)
  btype = 90;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [len], []);
endfunction



//
// A swich with two inputs and one output depending on a state. initial state=initial_state
// state 0 means the first input is feed through
// state 1 means the first input is feed through
//
// Switching occurs to state 0 if event 1 occurs
// Switching occurs to state 1 if event 2 occurs
// event 0 is the normal regular event
//

function [sim,bid] = libdyn_new_blk_2to1evsw(sim, events, initial_state)
  btype = 110;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [initial_state], []);
endfunction














//////////////////////////////////////////////////////////
// fancy shortcuts, that *should* be used by the user
//////////////////////////////////////////////////////////

function [sim,bid] = libdyn_new_blk_const(sim, events, c1)
  btype = 40;
  [sim,bid] = libdyn_new_block(sim, events, btype, [], [c1], ...
                   insizes=[], outsizes=[1], ...
                   intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]  );
endfunction
function [sim,c] = ld_const(sim, events, val) // PARSEDOCU_BLOCK
//
// %PURPOSE: A constant val
//

    [sim,c] = libdyn_new_blk_const(sim, events, val); // Instead of event a predefined initial event that only occurs once should be used
    [sim,c] = libdyn_new_oport_hint(sim, c, 0);    
endfunction



// FIXME no port size checking
function [sim,bid] = libdyn_new_blk_sum(sim, events, c1, c2)
  btype = 12;
  
  [sim,bid] = libdyn_new_block(sim, events, btype, [], [c1; c2], ...
                   insizes=[1,1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]);
  
//  [sim,bid] = libdyn_new_blockid(sim);
//  id = bid; // id for this parameter set
//  
//  sim.parlist = new_irparam_elemet(sim.parlist, id, IRPAR_LIBDYN_BLOCK, [btype; bid], [c1; c2]);
endfunction
function [sim,sum_] = ld_sum(sim, events, inp_list, fak1, fak2) 
// FIXME obsolete
    [sim,sum_] = libdyn_new_blk_sum(sim, events, fak1, fak2);
    [sim,sum_] = libdyn_conn_equation(sim, sum_, inp_list);  
    [sim,sum_] = libdyn_new_oport_hint(sim, sum_, 0);    
endfunction
function [sim,sum_] = ld_add(sim, events, inp_list, fak_list) // PARSEDOCU_BLOCK
//
// %PURPOSE: Add signals (linear combination)
// inp_list = list( in1, in2 )  ; fak_list = [ c1, c2 ]
// sum_ = in1 * c1 + in2 * c2
//

  ortd_checkpar(sim, list('SignalList', 'inp_list', inp_list) );
  ortd_checkpar(sim, list('Vector', 'fak_list', fak_list) );


    [sim,sum_] = libdyn_new_blk_sum(sim, events, fak_list(1), fak_list(2));
    [sim,sum_] = libdyn_conn_equation(sim, sum_, inp_list);  
    [sim,sum_] = libdyn_new_oport_hint(sim, sum_, 0);    
endfunction





// Multiplication 
// d1, d2: multiplicate (=0) or divide (=1) corresponding input; need exactly 2 inputs
function [sim,bid] = libdyn_new_blk_mul(sim, events, d1, d2)
  btype = 70;
  [sim,bid] = libdyn_new_block(sim, events, btype, [d1, d2], [], ...
                   insizes=[1,1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]);
endfunction
function [sim,mul_] = ld_mul(sim, events, inp_list, muldiv1, muldiv2)
// %PURPOSE: Multiplication 
// muldiv1/2: multiplicate (=0) or divide (=1) corresponding input; need exactly 2 inputs
 // FIXME obsolete
    [sim,mul_] = libdyn_new_blk_mul(sim, events, muldiv1, muldiv2);
    [sim,mul_] = libdyn_conn_equation(sim, mul_, inp_list);  
    [sim,mul_] = libdyn_new_oport_hint(sim, mul_, 0);    
endfunction
function [sim,mul_] = ld_dot(sim, events, inp_list, muldiv1_list)
// %PURPOSE: Multiplication 
// muldiv1/2: multiplicate (=0) or divide (=1) corresponding input; need exactly 2 inputs
// inp_list = list( in1, in2 )  ; muldiv1_list = [ muldiv1, muldiv2 ]
    [sim,mul_] = libdyn_new_blk_mul(sim, events, muldiv1_list(1), muldiv1_list(2) );
    [sim,mul_] = libdyn_conn_equation(sim, mul_, inp_list);  
    [sim,mul_] = libdyn_new_oport_hint(sim, mul_, 0);    
endfunction
function [sim,mul_] = ld_mult(sim, events, inp_list, muldiv1_list) // PARSEDOCU_BLOCK
//
// %PURPOSE: Multiplication 
// muldiv1/2: multiplicate (=0) or divide (=1) corresponding input; need exactly 2 inputs
// inp_list = list( in1, in2 )  ; muldiv1_list = [ muldiv1, muldiv2 ]
//
    [sim,mul_] = libdyn_new_blk_mul(sim, events, muldiv1_list(1), muldiv1_list(2) );
    [sim,mul_] = libdyn_conn_equation(sim, mul_, inp_list);  
    [sim,mul_] = libdyn_new_oport_hint(sim, mul_, 0);    
endfunction











function [sim,out] = ld_gain(sim, events, in, gain) // PARSEDOCU_BLOCK
//
// %PURPOSE: A simple gain
// 
// in * - input
// out * - output
// 
// out = in * gain
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'gain', gain) );
end

  [inp] = libdyn_extrakt_obj(in ); // compatibility

  btype = 20;
  [sim,bid] = libdyn_new_block(sim, events, btype, [], [gain], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,out] = libdyn_conn_equation(sim, bid, list(inp,0));
  [sim,out] = libdyn_new_oport_hint(sim, out, 0);    
endfunction








function [sim,sign_] = ld_sign(sim, events, inp_list, thr) // PARSEDOCU_BLOCK
//
// FIXME: OSOLETE FN: use ld_getsign instead
// 
// return the sign of the input sigal
// either 1 or -1
// 
// 
//
    [inp] = libdyn_extrakt_obj( inp_list ); // compatibility

    [sim,sign_] = libdyn_new_compare(sim, events, thr);
    [sim,sign_] = libdyn_conn_equation(sim, sign_, list(inp));
    [sim,sign_] = libdyn_new_oport_hint(sim, sign_, 0);
endfunction











function [sim,lkup] = ld_lkup(sim, events, inp_list, lower_b, upper_b, table)
// %PURPOSE: lookup table
//
// inp_list - input signal
// table - a vector of values to look up
// the input is mapped between lower_b and upper_b to the
// index within table
  [inp] = libdyn_extrakt_obj( inp_list ); // compatibility

    [sim,lkup] = libdyn_new_blk_lkup(sim, events, lower_b, upper_b, table);
    [sim,lkup] = libdyn_conn_equation(sim, lkup, list(inp));
    [sim,lkup] = libdyn_new_oport_hint(sim, lkup, 0);
endfunction
  
  
  
  
  
  
  
function [sim,out] = ld_fngen(sim, events, shape_, period, amp) // PARSEDOCU_BLOCK
//
// %PURPOSE: function generator
// 
// shape_ - the shape of the output signal: =0 : sinus, more to come...
// period, amp * - Periode length in samples and amplitude
// out * - output
// 
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('SingleValue', 'shape_', shape_) );
end

  btype = 80;
  [sim,bid] = libdyn_new_block(sim, events, btype, [shape_], [], ...
                   insizes=[1,1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,out] = libdyn_conn_equation(sim, bid, list(period, amp));
  [sim,out] = libdyn_new_oport_hint(sim, out, 0);
endfunction

  
  


// function [sim,delay] = ld_delay(sim, events, inp_list, delay_len)
//     [inp] = libdyn_extrakt_obj( inp_list ); // compatibility
// 
//     [sim,delay] = libdyn_new_delay(sim, events, delay_len)
//     [sim,delay] = libdyn_conn_equation(sim, delay, list(inp));
//     [sim,delay] = libdyn_new_oport_hint(sim, delay, 0);    
// endfunction












// FIXME no port size checking
function [sim,bid] = libdyn_new_blk_zTF(sim, events, H)
  btype = 30;
  bip = [ degree(H.num); degree(H.den) ];
  brp = [ coeff(H.num)'; coeff(H.den)' ];

  [sim,bid] = libdyn_new_block(sim, events, btype, [bip], [brp], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]);
  
//  [sim,bid] = libdyn_new_blockid(sim);
//  id = bid; // id for this parameter set
//  
//  
//  sim.parlist = new_irparam_elemet(sim.parlist, id, IRPAR_LIBDYN_BLOCK, [btype; bid; bip], [brp]);
endfunction
function [sim,y] = ld_ztf(sim, events, inp_list, H) // PARSEDOCU_BLOCK
//
// %PURPOSE: Time discrete transfer function
// H is give as a Scilab rational
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'inp_list', inp_list) );
end

  [inp] = libdyn_extrakt_obj( inp_list ); // compatibility

    [sim,tf] = libdyn_new_blk_zTF(sim, events, H);
    [sim,y] = libdyn_conn_equation(sim, tf, list(inp));
    [sim,y] = libdyn_new_oport_hint(sim, y, 0);    
endfunction










function [sim,bid] = libdyn_new_blk_sat(sim, events, lowerlimit, upperlimit)
  btype = 50;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [], [lowerlimit, upperlimit], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]);
endfunction
function [sim,y] = ld_sat(sim, events, inp_list, lowerlimit, upperlimit) // PARSEDOCU_BLOCK
//
// %PURPOSE: Saturation between lowerlimit and upperlimit
//


  [inp] = libdyn_extrakt_obj( inp_list ); // compatibility

    [sim,sat] = libdyn_new_blk_sat(sim, events, lowerlimit, upperlimit);
    [sim,y] = libdyn_conn_equation(sim, sat, list(inp));
    [sim,y] = libdyn_new_oport_hint(sim, y, 0);    
endfunction










function [sim,bid] = libdyn_new_flipflop(sim, events, initial_state)
  btype = 160;
  [sim,bid] = libdyn_new_block(sim, events, btype, [initial_state], [], ...
                   insizes=[1,1,1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]);
endfunction
function [sim,y] = ld_flipflop(sim, events, set0, set1, reset, initial_state) // PARSEDOCU_BLOCK
//
// %PURPOSE: A flip-flop
//
// set0, set1, reset * - control of the flipflop (set output to zero if set0 is >0.5 for at least one sample, ...)
// initial_state - constant
//

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'set0', set0) );
  ortd_checkpar(sim, list('Signal', 'set1', set1) );
  ortd_checkpar(sim, list('Signal', 'reset', reset) );
  ortd_checkpar(sim, list('SingleValue', 'init_state', init_state) );
end

    [sim,blk] = libdyn_new_flipflop(sim, events, initial_state);
    [sim,blk] = libdyn_conn_equation(sim, blk, list(set0,0, set1,0, reset,0)); // FIXME: remove ,0
    [sim,y] = libdyn_new_oport_hint(sim, blk, 0);    
endfunction




// function [sim,bid] = libdyn_new_printf(sim, events, str, insize) //REMOVE
//   btype = 170;
//   str = ascii(str);
//   [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [insize, length(str), str(:)'], []);
// endfunction



//
// Some terminal color codes for usage with ld_printf  MOVED TO libdyn.sci as a workaround
//

// ORTD.termcode.red = ascii(27) + '[31m';
// ORTD.termcode.green = ascii(27) + '[32m';
// ORTD.termcode.yellow = ascii(27) + '[33m';
// ORTD.termcode.blue = ascii(27) + '[34m';
// ORTD.termcode.reset = ascii(27) + '[0m';


function [sim] = ld_printf(sim, events, in, str, insize) // PARSEDOCU_BLOCK
//
// %PURPOSE: Print data to stdout (the console)
//
// in *+(insize) - vectorial input signal
//
// str is a string that is printed followed by the signal vector in
// of size insize
//
// Hint: Apply colored printf's by using the predefined terminal color codes:
// 
// str = ORTD.termcode.red + "some colored text..." + ORTD.termcode.reset
// 
// instead of red there currently is: green, yellow, blue.
// 
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'insize', insize) );
  ortd_checkpar(sim, list('String', 'str', str) );
end

  btype = 170;
  str = ascii(str);
//   [sim,blk] = libdyn_new_blk_generic(sim, events, btype, [insize, length(str), str(:)'], []);

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ insize, length(str), str(:)' ], rpar=[ ], ...
                   insizes=[ insize ], outsizes=[], ...
                   intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[]  );


  [sim,blk] = libdyn_conn_equation(sim, blk, list(in,0) );
endfunction

function [sim] = ld_printfInt32(sim, events, in, str, insize) // PARSEDOCU_BLOCK
//
// %PURPOSE: Print data to stdout (the console)
//
// in *+(insize) - vectorial input signal (Int32)
//
// str is a string that is printed followed by the signal vector in
// of size insize
//
// Hint: Apply colored printf's by using the predefined terminal color codes:
// 
// str = ORTD.termcode.red + "some colored text..." + ORTD.termcode.reset
// 
// instead of red there currently is: green, yellow, blue.
// 
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'insize', insize) );
  ortd_checkpar(sim, list('String', 'str', str) );
end

  btype = 60001 + 78;
  str = ascii(str);
//   [sim,blk] = libdyn_new_blk_generic(sim, events, btype, [insize, length(str), str(:)'], []);

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ insize, length(str), str(:)' ], rpar=[ ], ...
                   insizes=[ insize ], outsizes=[], ...
                   intypes=[ ORTD.DATATYPE_INT32 ], outtypes=[]  );


  [sim,blk] = libdyn_conn_equation(sim, blk, list(in,0) );
endfunction

function [sim] = ld_printfBin(sim, events, in, str, insize) // PARSEDOCU_BLOCK
//
// %PURPOSE: Print data to stdout (the console)
//
// in *+(insize) - vectorial input signal (Binaray)
//
// str is a string that is printed followed by the signal vector in
// of size insize
//
// Hint: Apply colored printf's by using the predefined terminal color codes:
// 
// str = ORTD.termcode.red + "some colored text..." + ORTD.termcode.reset
// 
// instead of red there currently is: green, yellow, blue.
// 
// 
// 

if ORTD.FASTCOMPILE==%f then
  ortd_checkpar(sim, list('Signal', 'in', in) );
  ortd_checkpar(sim, list('SingleValue', 'insize', insize) );
  ortd_checkpar(sim, list('String', 'str', str) );
end

  btype = 60001 + 93;
  str = ascii(str);
//   [sim,blk] = libdyn_new_blk_generic(sim, events, btype, [insize, length(str), str(:)'], []);

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ insize, length(str), str(:)' ], rpar=[ ], ...
                   insizes=[ insize ], outsizes=[], ...
                   intypes=[ ORTD.DATATYPE_BINARY ], outtypes=[]  );


  [sim,blk] = libdyn_conn_equation(sim, blk, list(in,0) );
endfunction



//
// A switching Block
// inputs = [control_in, signal_in]
// if control_in > 0 : signal_in is directed to output 1; output_2 is set to zero
// if control_in < 0 : signal_in is directed to output 2; output_1 is set to zero
//
function [sim,bid] = libdyn_new_blk_switch(sim, events)
  btype = 60;
  [sim,bid] = libdyn_new_block(sim, events, btype, [], [], ...
                   insizes=[1,1], outsizes=[1,1], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT]);
endfunction
function [sim,out_1, out_2] = ld_switch(sim, events, cntrl, in) // PARSEDOCU_BLOCK
//
// %PURPOSE: A switching Block
// inputs = [control_in, signal_in]
// if control_in > 0 : signal_in is directed to output 1; output_2 is set to zero
// if control_in < 0 : signal_in is directed to output 2; output_1 is set to zero
//
    [sim,blk] = libdyn_new_blk_switch(sim, events);
    [sim,blk] = libdyn_conn_equation(sim, blk, list(cntrl,0, in,0));
    
    [sim,out_1] = libdyn_new_oport_hint(sim, blk, 0);
    [sim,out_2] = libdyn_new_oport_hint(sim, blk, 1);
endfunction










//
// Sample play block
//
// plays the sequence stored in r
// each time event 0 occurs the next value of r is put out
// sampling start either imediadedly (initial_play=1) or on event 1.
// Event 2 stops sampling and set ouput to last values (mute_afterstop = 0 and hold_last_values == 1) 
// or zero (mute_afterstop = 1)
//
function [sim,bid] = libdyn_new_blk_play(sim, events, r, initial_play, hold_last_value, mute_afterstop)
  if (exists('initial_play') ~= 1) then
    initial_play = 1;  
  end
  if (exists('hold_last_value') ~= 1) then
    hold_last_value = 0;  
  end
  if (exists('mute_afterstop') ~= 1) then
    mute_afterstop = 0;  
  end
  
  btype = 100;
  [sim,bid] = libdyn_new_block(sim, events, btype, [length(r), initial_play, hold_last_value, mute_afterstop], [r], ...
                   insizes=[], outsizes=[1], ...
                   intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]);
endfunction
function [sim,y] = ld_play_simple(sim, events, r) // PARSEDOCU_BLOCK
//
// %PURPOSE: Simple sample play block
//
// plays the sequence stored in r
// each time event 0 occurs the next value of r is put out
//

  [sim,y] = libdyn_new_blk_play(sim, events, r, 1, 1, 0);
  [sim,y] = libdyn_new_oport_hint(sim, y, 0);    
endfunction










// Dump at max maxlen samples to file "filename"; start automatically if autostart == 1
function [sim,bid] = libdyn_new_blk_filedump(sim, events, filename, vlen, maxlen, autostart)
  btype = 130;
  fname = ascii(filename);
  [sim,bid] = libdyn_new_block(sim, events, btype, [maxlen, autostart, vlen, length(fname), fname(:)'], [], ...
                   insizes=[vlen], outsizes=[], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[]);
endfunction
function [sim,save_]=libdyn_dumptoiofile(sim, events, fname, source) //OBSOLET
// %PURPOSE: Quick and easy dumping of signals to files in one line of code
// obsolete version
  [source] = libdyn_extrakt_obj( source ); // compatibility

  // source: a list with a block + a port
  [sim,save_] = libdyn_new_blk_filedump(sim, events, fname, 1, 0, 1);
  [sim,save_] = libdyn_conn_equation(sim, save_, list(source, 0) );
endfunction
function [sim,save_]=ld_dumptoiofile(sim, events, fname, source) //OBSOLET
//
// Quick and easy dumping of signals to files
// source - signal of size 1 (at the moment)
// fname - filename string
//
  [inp] = libdyn_extrakt_obj( source ); // compatibility

  // source: a list with a block + a port
  [sim,save_] = libdyn_new_blk_filedump(sim, events, fname, 1, 0, 1);
  [sim,save_] = libdyn_conn_equation(sim, save_, list(source) );
endfunction




//////////////////////////////////////////////////////////
// More complex blocks based on elementary blocks
//////////////////////////////////////////////////////////

//
// Generic controllers
//

function [sim,u] = ld_standard_controller(sim, event, r, y, K)
// classic linear controller
    [sim,e] = ld_sum(sim, event, list(r,0, y,0), 1,-1 );
    [sim,u] = ld_ztf(sim, event, list(e,0), K);
endfunction

function [sim,u] = ld_standard_controller_2dof(sim, event, r, y, K, M)
// like classic controller but with a measurement filter M
    [sim,y_] = ld_ztf(sim, event, list(y,0), M);
    [sim,e] = ld_sum(sim, event, list(r,0, y_,0), 1,-1 );
    [sim,u] = ld_ztf(sim, event, list(e,0), K);
endfunction

function [sim,u,u_fb,u_ff] = ld_standard_controller_ffw(sim, event, r, y, K, Gm1, T)
// controller with a feedforwad part
    [sim,r_] = ld_ztf(sim, event, list(r,0), T); // closed loop model
    
    [sim,e] = ld_sum(sim, event, list(r_,0, y,0), 1,-1 );
    [sim,u_fb] = ld_ztf(sim, event, list(e,0), K);
    
    [sim,u_ff] = ld_ztf(sim, event, list(r,0), Gm1);
    [sim,u] = ld_sum(sim, event, list(u_fb,0, u_ff,0), 1,1 );
endfunction


