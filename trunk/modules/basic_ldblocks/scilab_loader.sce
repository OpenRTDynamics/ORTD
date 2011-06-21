

// Interfacing functions are placed in this place



//
// A 2 to 1 switching Block
// inputs = [control_in, signal_in1, signal_in2]
// if control_in > 0 : out = signal_in1
// if control_in < 0 : out = signal_in2
//
function [sim,out] = ld_switch2to1(sim, events, cntrl, in1, in2)
  btype = 60001;
  [sim,blk] = libdyn_new_blk_generic(sim, events, btype, [], []);

  [sim,blk] = libdyn_conn_equation(sim, blk, list(cntrl, in1, in2) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction





function [sim,outlist] = ld_demux(sim, events, vecsize, invec)
  btype = 60001 + 1;	
  ipar = [vecsize, 0]; rpar = [];
  [sim,blk] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar);

  [sim,blk] = libdyn_conn_equation(sim, blk, list(invec) );

  // connect each outport
  outlist = list();
  for i = 1:vecsize
    [sim,out] = libdyn_new_oport_hint(sim, blk, i-1);   // ith port
    outlist(i) = out;
  end
endfunction



function [sim,out] = ld_mux(sim, events, vecsize, inlist)
  btype = 60001 + 2;	
  ipar = [vecsize; 0]; rpar = [];
//pause;
  [sim,blk] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar);
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, inlist );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction




//
// Macros
//
//





// Implements a time discrete integrator with saturation of the output between min__ and max__
// y(k+1) = sat( y(k) + Ta*u , min__, max__ )
function [sim, y] = ld_limited_integrator(sim, ev, u, min__, max__, Ta)
    [sim, u__] = ld_gain(sim, ev, u, Ta);
    
    [sim,z_fb] = libdyn_new_feedback(sim);
    
    [sim, sum_] = ld_sum(sim, ev, list(u__, z_fb), 1, 1);
    [sim, tmp] = ld_ztf(sim, ev, sum_, 1/z);
    [sim, y] = ld_sat(sim, ev, tmp, min__, max__);
    
    [sim] = libdyn_close_loop(sim, y, z_fb);    
endfunction


// linear controller with anti reset windup implemented by bounding the integral state:
// e = r-y
// u = ld_limited_integrator( e, min__, max__ ) + tfR*e
function [sim, u] = ld_lin_awup_controller(sim, ev, r, y, Ta, tfR, min__, max__)
    [sim, e] = ld_sum(sim, ev, list(r, y), 1, -1);
    
    [sim,u1] = ld_limited_integrator(sim, ev, e, min__, max__, Ta);
    [sim,u2] = ld_ztf(sim, ev, e, tfR);
    
    [sim,u] = ld_sum(sim, ev, list(u1,u2), 1,1);
endfunction


