// function [sim, out] = ld_AndroidSensors(sim, events, in, rate) // PARSEDOCU_BLOCK
// // %PURPOSE: Read out Android Sensors and synchronise the simulation to them
// //
// // Special: SYNC_BLOCK (use only one block of this type in an asynchronous running sub-simulation)
// //
// // out - vector of size 10 containing the sensor values
// // in - when in becomes one, the synchronisation loop is interrupted
// // 
// 
// 
//  btype = 15500 + 0; //
//  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ 0  ], rpar=[ rate ], ...
//                   insizes=[1], outsizes=[10], ...
//                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );
// 
//  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
//  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
// endfunction
// 
// 
// 


function [sim, out, SensorID] = ld_AndroidSensors(sim, events, in, ConfigStruct) // PARSEDOCU_BLOCK
// %PURPOSE: Read out Android Sensors and synchronise the simulation to them
//
// Special: SYNC_BLOCK (use only one block of this type in an asynchronous running sub-simulation)
//
// out - vector of size 10 containing the sensor values
// in - when in becomes one, the synchronisation loop is interrupted
// SensorID - The ID of the sensor that send new data
// 
// Possible sensor ID's:
// 
//     ASENSOR_TYPE_ACCELEROMETER      = 1,
//     ASENSOR_TYPE_MAGNETIC_FIELD     = 2,
//     ASENSOR_TYPE_GYROSCOPE          = 4,
//     ASENSOR_TYPE_LIGHT              = 5,
//     ASENSOR_TYPE_PROXIMITY          = 8
// 
// EXPERIMENTAL
// 




  printf("Including Android sensor block\n");	


  try
    ConfigStruct.rateAcc;
  catch
    ConfigStruct.rateAcc = 0;
  end

  try
    ConfigStruct.rateGyro; 
  catch
    ConfigStruct.rateGyro = 0;
  end

  try
    ConfigStruct.rateMagn; 
  catch
    ConfigStruct.rateMagn = 0;
  end

  try
    ConfigStruct.rateGPS;
  catch
    ConfigStruct.rateGPS = 0;
  end


// introduce some parameters that are refered to by id's
// 
    // pack all parameters into a structure "parlist"
    parlist = new_irparam_set();
 
    parlist = new_irparam_elemet_ivec(parlist, [ ConfigStruct.rateAcc; ConfigStruct.rateGyro; ConfigStruct.rateMagn; ConfigStruct.rateGPS ] , 10); // id = 10
//    parlist = new_irparam_elemet_ivec(parlist, vec, 11); // vector of integers (double vectors are similar, replace ivec with rvec)
//    parlist = new_irparam_elemet_ivec(parlist, ascii(str), 12); // id = 12; A string parameter
// 
    p = combine_irparam(parlist); // convert to two vectors of integers and floating point values respectively



// Set-up the block parameters and I/O ports
  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 15500 + 0; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[1]; // Input port sizes
  outsizes=[10, 1]; // Output port sizes
  dfeed=[1];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=[ORTD.DATATYPE_FLOAT]; // datatype for each input port
  outtypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_INT32]; // datatype for each output port

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC (if block uses states), 2-BLOCKTYPE_STATIC (if there is only a static relationship between in- and output)

  // Create the block
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed);
  
  // connect the inputs
 [sim,blk] = libdyn_conn_equation(sim, blk, list(in) ); // connect in1 to port 0 and in2 to port 1

  // connect the ouputs
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
 [sim,SensorID] = libdyn_new_oport_hint(sim, blk, 1);   // 0th port


//     ASENSOR_TYPE_ACCELEROMETER      = 1,
//     ASENSOR_TYPE_MAGNETIC_FIELD     = 2,
//     ASENSOR_TYPE_GYROSCOPE          = 4,
//     ASENSOR_TYPE_LIGHT              = 5,
//     ASENSOR_TYPE_PROXIMITY          = 8

endfunction

