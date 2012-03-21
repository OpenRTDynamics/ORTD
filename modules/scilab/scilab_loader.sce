

// Interfacing functions are placed into this place



function [sim, out] = ld_scilab(sim, events, in, invecsize, outvecsize,  ...
    init_cmd, calc_cmd, destruct_cmd, scilab_path) // PARSEDOCU_BLOCK
// scilab - block
//
// in *+(invecsize) - input        scilab_interf.invec%d = [ .... ];
// out *+(outvecsize) - output        scilab_interf.outvec%d = [ .... ];
// 
// out = calc_cmd        scilab_interf.outvec%d = calc_cmd(scilab_interf.invec%d);
// 


  invecno=1; outvecno=1;

  btype = 22000;
  ipar = [invecsize; outvecsize; invecno; outvecno; 0; 0; 0; 0; 0; 0; 0; length(init_cmd); length(calc_cmd); ...
                   length(destruct_cmd); length(scilab_path); ascii(init_cmd)';...
                   ascii(calc_cmd)'; ascii(destruct_cmd)'; ascii(scilab_path)'; 0; ]; rpar = [];
  
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                   insizes=[invecsize], outsizes=[outvecsize], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction
