

// Interfacing functions are placed in this place


function [sim,out] = ld_serial_interface(sim, events, in, devname, send_cmd, rcv_cmd, inSize, outSize, BaudRate)
// 
// 
// 
// 
// 
// 
// 

  btype = 16001;

  devname_as = ascii(devname);
  send_cmd_as = ascii(send_cmd);
  rcv_cmd_as = ascii(rcv_cmd);
  
  ipar = [0, inSize, outSize, BaudRate, 0, 0, 0, length(devname_as), length(send_cmd_as), length(rcv_cmd_as),  devname_as(:)', send_cmd_as(:)', rcv_cmd_as(:)', ];
  rpar = [ ];
  
  [sim,blk] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar);


  [sim,blk] = libdyn_conn_equation(sim, blk, list(in));
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

