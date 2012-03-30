

// Interfacing functions are placed in this place

function [sim, out] = ld_udp_main_receiver(sim, events, udpport, identstr, socket_fname, vecsize) // PARSEDOCU_BLOCK
// udp main receiver - block
//
// 

  datatype = ORTD.DATATYPE_FLOAT;

  btype = 39001;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ udpport, vecsize, datatype, length(socket_fname), ascii(socket_fname), length(identstr), ascii(identstr) ], rpar=[  ], ...
                   insizes=[], outsizes=[vecsize], ...
                   intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]  );

  //[sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

