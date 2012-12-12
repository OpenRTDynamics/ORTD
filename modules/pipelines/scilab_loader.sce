function [sim] = ld_ringbuf(sim, events, ident_str, datatype, len, visibility)  // PARSEDOCU_BLOCK
// 
// %PURPOSE: inittialise a new ringbuffer
// 
// ident_str (string) - name of the ringbuffer
// datatype - ORTD datatype of the ringbuffer (for now only ORTD.DATATYPE_FLOAT)
// len (integer) - number of elements
// initial_data - initial data of the ringbuffer
// visibility (string) - 'global', ... (more are following)
// 
// 

  ident_str = ident_str + '.ringbuf';

  if (visibility == 'global') then
  
  else
    error("Visibility has to be one of global, ... (more are following)");
  end

  UNUSED = 0;

  btype = 15400 + 0   ;	
  ipar = [0, datatype, len, UNUSED, 0,0,0,0, 0,0, length(ident_str), ascii(ident_str) ]; 
  
  rpar = [ ];

  if datatype == ORTD.DATATYPE_FLOAT then
//     rpar = [ initial_data ];
//     if (length(initial_data) ~= len) then
//       error("length(initial_data) ~= len");
//     end
  else
    rpar = [ ];
    error("datatype is not one of ORTD.DATATYPE_FLOAT, ...");
  end

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                   insizes=[], outsizes=[], ...
                   intypes=[], outtypes=[]  );
 
 // ensure the block is included in the simulation even without any I/O ports
 sim = libdyn_include_block(sim, blk);
endfunction

function [sim] = ld_write_ringbuf(sim, events, data, ident_str, datatype, ElementsToWrite)   // PARSEDOCU_BLOCK
// 
// %PURPOSE: Write a portion to a ringbuffer
// 
// 
// data *+(ElementsToWrite) - data
// ident_str (string) - name of the memory
// datatype - ORTD datatype of the memory (for now only ORTD.DATATYPE_FLOAT)
// ElementsToWrite (integer) - number of elements to write to the memory
// 
// 

  ident_str = ident_str + '.ringbuf';

  btype = 15400 + 1   ;	
  ipar = [0, datatype, ElementsToWrite, 0, 0,0,0,0, 0,0, length(ident_str), ascii(ident_str) ]; 
  rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                   insizes=[ElementsToWrite ], outsizes=[], ...
                   intypes=[ datatype ], outtypes=[]  );
 
 
 [sim,blk] = libdyn_conn_equation(sim, blk, list(data) );
 
 
 
//   [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, data, NumRead] = ld_read_ringbuf(sim, events, ident_str, datatype, ElementsToRead)   // PARSEDOCU_BLOCK
// 
// %PURPOSE: Read a portion from a ringbuffer
// 
// data *+(ElementsToRead) - data
// ident_str (string) - name of the ringbuffer
// datatype - ORTD datatype of the ringbuffer (for now only ORTD.DATATYPE_FLOAT)
// ElementsToRead (integer) - number of elements to read from the ringbuffer
// 

  ident_str = ident_str + '.ringbuf';

  btype = 15400 + 2;
  ipar = [0, datatype, ElementsToRead, 0, 0,0,0,0, 0,0, length(ident_str), ascii(ident_str) ]; 
  rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                   insizes=[ ], outsizes=[ElementsToRead, 1], ...
                   intypes=[ ], outtypes=[datatype, ORTD.DATATYPE_FLOAT ]  );
 
 // ensure the block is included in the simulation even without any I/O ports
//  [sim,blk] = libdyn_conn_equation(sim, blk, list(index) );
 
   [sim,data] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
   [sim,NumRead] = libdyn_new_oport_hint(sim, blk, 1);   // 1th port 
   
endfunction

