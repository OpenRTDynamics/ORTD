

// Interfacing functions are placed in this place



function [sim,out] = ld_parameter(sim, events, str, initial_param)
  // Creates a new parameter block that is remotely controlable via TCP
  // It requires the set-up of a libdyn master
  // 
  // str - is a string of the parameter name
  // initial_param - is a vector of the initial parameter set
  // out - is a vectorial signal of size length(initial_param)
  // 
  Nin = 0;
  Nout = 1;

  btype = 14001;
  str = ascii(str);
  nparam = length(initial_param);
  
  ipar = [0, nparam, length(str), str(:)'];
  rpar = [initial_param(:)];
  
  [sim,blk] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar);


//   [sim,blk] = libdyn_conn_equation(sim, blk, inlist);
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim] = ld_stream(sim, events, in, str, insize)
  // Creates a new stream block that is remotely controlable via TCP
  // It requires the set-up of a libdyn master
  // 
  // str - is a string of the stream name
  // insize is the vector length of the input port
  // 
  bufferlen = 100; // how many vectors should be stored in the ringbuffer
  datatype = -1; // FIXME set to FLOAT

  btype = 14001 + 1;
  str = ascii(str);

//     int insize = ipar[1];    
//     datatype = ipar[2];
//     datatype = DATATYPE_FLOAT; // FIXME REMOVE
// 
//     int bufferlen = ipar[3];
//     int exprlen = ipar[4];
//     int *coded_stream_name = &ipar[5];

  
  ipar = [0, insize, datatype, bufferlen, length(str), str(:)'];
  rpar = [];
  
  [sim,blk] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar);

  // connect input port
  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
endfunction