//
//    Copyright (C) 2010, 2011  Christian Klauer
//
//    This file is part of OpenRTDynamics, the Real Time Dynamic Toolbox
//
//    OpenRTDynamics is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    OpenRTDynamics is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with OpenRTDynamics.  If not, see <http://www.gnu.org/licenses/>.
//





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


function [sim] = ld_stream(sim, events, in, str, insize, autoflushInterval, autoflushTimeout, bufferlen)
  // Creates a new stream block that is remotely controlable via TCP
  // It requires the set-up of a libdyn master
  // 
  // str - is a string of the stream name
  // insize is the vector length of the input port
  // [autoflushInterval] how often (in samples) to flush the send buffers
  // [autoflushTimeout] how often to flush the send buffers (time difference) (not implemented)
  // [bufferlen] number of samples within the ringbuffer
  //

  // set some defeult values for optional variables if not present
  if (exists('bufferlen') == 0)
    bufferlen = 1000; // how many vectors should be stored in the ringbuffer
  end

  if (exists('autoflushInterval') == 0)
    autoflushInterval = 1;
  end

  if (exists('autoflushTimeout') == 0)
    autoflushTimeout = 0.1;
  end

  datatype = -1; // FIXME set to FLOAT


  btype = 14001 + 1;
  str = ascii(str);

  
  ipar = [0, insize, datatype, bufferlen, autoflushInterval, length(str), str(:)'];
  rpar = [autoflushTimeout ];
  
  [sim,blk] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar);

  // connect input port
  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
endfunction