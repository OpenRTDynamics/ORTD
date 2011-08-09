

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
  [sim,blk] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar, ...
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



function [sim,out] = ld_mux(sim, events, vecsize, inlist)
  btype = 60001 + 2;	
  ipar = [vecsize; 0]; rpar = [];
//pause;
  [sim,blk] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar);
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, inlist );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim,out] = ld_hysteresis(sim, events, in, switch_on_level, switch_off_level, initial_state, onout, offout)
// hysteresis - block
//
// switches out between onout and offout
// initial state is either -1 (off) or 1 (on)
//
//

  if (switch_off_level > switch_on_level) then
    error("ld_hysteresis: setting switch_off_level > switch_on_level makes no sense\n");
  end

  btype = 60001 + 3;
  [sim,blk] = libdyn_new_blk_generic(sim, events, btype, [initial_state], [ switch_on_level, switch_off_level, onout, offout]);

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim,out] = ld_modcounter(sim, events, in, initial_count, mod)
// Modulo Counter - Block
//
// A counter that increases its value for each timestep where in > 0.
// if the counter value >= mod then it is reset to counter = initial_count
//
//

  if (mod < 0) then
    error("ld_modcounter: mod is less than zero\n");
  end

  btype = 60001 + 4;
  [sim,blk] = libdyn_new_blk_generic(sim, events, btype, [ initial_count, mod ], [  ]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim,out] = ld_jumper(sim, events, in, steps)
// hysteresis - block
//
// switches out between onout and offout
// initial state is either -1 (off) or 1 (on)
//
//

  if (steps <= 0) then
    error("ld_jumper: steps must be greater than zero\n");
  end

  btype = 60001 + 5;
  [sim,blk] = libdyn_new_blk_generic(sim, events, btype, [ steps ], [  ]);

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



//
// Macros
//
//


function [sim, y] = ld_mute( sim, ev, u, cntrl, mutewhengreaterzero )
//    
//    Mute a signal based on cntrl-signal
//
//    ev - event
//    u - input
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


// Convert an angle in rad to degree and print to console
function [sim] = ld_print_angle(sim, ev, alpha, text)
    [sim, alpha_deg] = ld_gain(sim, ev, alpha, 1/%pi*180);
    [sim] = ld_printf(sim, ev, alpha_deg, text, 1);
endfunction
