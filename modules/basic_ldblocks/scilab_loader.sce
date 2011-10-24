

// Interfacing functions are placed in this place


function [sim]=ld_savefile(sim, events, fname, source, vlen) // PARSEDOCU_BLOCK
// Quick and easy dumping of signals to files
// 
// fname - string of the filename
// source - source signal
// vlen - vector size of signal
// 

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


function [sim,out] = ld_switch2to1(sim, events, cntrl, in1, in2) // PARSEDOCU_BLOCK
//
// A 2 to 1 switching Block
// inputs = [control_in, signal_in1, signal_in2]
// if control_in > 0 : out = signal_in1
// if control_in < 0 : out = signal_in2
//
  btype = 60001;
  [sim,blk] = libdyn_new_block(sim, events, btype, [], [], ...
                   insizes=[1, 1, 1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], ...
                   outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(cntrl, in1, in2) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction





function [sim,outlist] = ld_demux(sim, events, vecsize, invec) // PARSEDOCU_BLOCK
//
// Demultiplexer
//
// Splits the input vector signal "invec" of size "vecsize" up into 
//
// outlist(1)
// outlist(1)
//  ....
//    
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



function [sim,out] = ld_mux(sim, events, vecsize, inlist) // PARSEDOCU_BLOCK
//    
// Multiplexer
//
// combines inlist(1), inlist(2), ...    
// to a vector signal "out" of size "vecsize"
//    
  btype = 60001 + 2;	
  ipar = [vecsize; 0]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[ones(1,vecsize)], outsizes=[vecsize], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT*ones(1,vecsize) ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, inlist );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim,out] = ld_hysteresis(sim, events, in, switch_on_level, switch_off_level, initial_state, onout, offout) // PARSEDOCU_BLOCK
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
  [sim,blk] = libdyn_new_block(sim, events, btype, [initial_state], [ switch_on_level, switch_off_level, onout, offout] , ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim,out] = ld_modcounter(sim, events, in, initial_count, mod) // PARSEDOCU_BLOCK
// Modulo Counter - Block
//
// A counter that increases its value for each timestep for which in > 0 is true.
// if the counter value >= mod then it is reset to counter = initial_count
//
//

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


function [sim,out] = ld_jumper(sim, events, in, steps) // PARSEDOCU_BLOCK
// jumper - block
//
// out - vector of size steps
// in - switching input
//
// The vector out always contains one "1", the rest is zero.
// The "1" moves to the right if in > 0. If the end is reached
// it "1" flips back to the left side
// 
// 

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


function [sim,out] = ld_memory(sim, events, in, rememberin, initial_state) // PARSEDOCU_BLOCK
// memory - block
//
// If rememberin > 0 then
//   remember in, which is then feed to the output out until it is overwritten by a new value
//
// initial output out = initial_state
// 

  btype = 60001 + 6;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [ initial_state ],  ...
                   insizes=[1, 1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]   );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in, rememberin) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim,out] = ld_abs(sim, events, in) // PARSEDOCU_BLOCK
// abs - block
//
// out = abs(in)
// 

  btype = 60001 + 7;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim,out] = ld_extract_element(sim, events, invec, pointer, vecsize ) // PARSEDOCU_BLOCK
  //
  // Extract one element of a vector
  //
  // invec - the input vector signal
  // pointer - the index signal
  // vecsize - length of input vector
  // 
  // out = invec[pointer], the first element is at pointer = 1
  //

  btype = 60001 + 8;	
  ipar = [ vecsize, ORTD.DATATYPE_FLOAT ]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[vecsize], outsizes=[1], ...
                       intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

//   [sim,blk] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar               );


  [sim,blk] = libdyn_conn_equation(sim, blk, list(invec, pointer) );

  // connect each outport
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // ith port
endfunction


function [sim,out] = ld_constvec(sim, events, vec) // PARSEDOCU_BLOCK
// 
// a constant vector
// 
// vec - the vector
// 
  btype = 60001 + 9;	
  ipar = [length(vec); 0]; rpar = [vec];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                   insizes=[1], outsizes=[ length(vec) ], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );
 
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim,out] = ld_counter(sim, events, count, reset, resetto, initial) // PARSEDOCU_BLOCK
// 
// A resetable counter block
//
// count - signal
// reset - signal
// resetto - signal
// initial - constant
// 
// increases out by count (out = out + count)
// 
// if reset > 0 then
//   out = resetto
//
// initially out is set to initial
// 
// 

  btype = 60001 + 10;
  ipar = [  ]; rpar = [ initial ];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[1,1,1], outsizes=[1], ...
                       intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], ...
                       outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list( count, reset, resetto ) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction




// 
//  Vector functions
// 


function [sim,out] = ld_vector_diff(sim, events, in, vecsize)
//    
// Vector differentiation
//
//    
  btype = 60001 + 50;	
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize], outsizes=[vecsize-1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,index] = ld_vector_findthr(sim, events, in, thr, greater, vecsize)
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

function [sim,out] = ld_vector_abs(sim, events, in, vecsize)
//    
// Vector abs()
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

function [sim,out] = ld_vector_gain(sim, events, in, gain, vecsize)
//    
// Vector gain
//
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

function [sim,out] = ld_vector_extract(sim, events, in, from, window_len, vecsize)
//    
// Extract "in" from to from+window_len
// 
//  in - vector signal
//  from - index signal
//
//    
  btype = 60001 + 54;	
  ipar = [vecsize; window_len]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize], outsizes=[window_len], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in, from ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim,out] = ld_vector_minmax(sim, events, in, findmax, vecsize)
//    
// Min / Max of a vector
//
//    
  btype = 60001 + 55;	
  ipar = [vecsize; findmax]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize], outsizes=[1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim,out] = ld_vector_addscalar(sim, events, in, add, vecsize)
//    
// add "add" to the vector
// 
//  add - signal
//  in - vector signal
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

function [sim,out] = ld_vector_sum(sim, events, in, vecsize)
//    
// sum over "in"
//
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




//
// Macros
//
//


function [sim, y] = ld_mute( sim, ev, u, cntrl, mutewhengreaterzero ) // PARSEDOCU_BLOCK
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



function [sim, y] = ld_limited_integrator(sim, ev, u, min__, max__, Ta) // PARSEDOCU_BLOCK
// Implements a time discrete integrator with saturation of the output between min__ and max__
// y(k+1) = sat( y(k) + Ta*u , min__, max__ )
    [sim, u__] = ld_gain(sim, ev, u, Ta);
    
    [sim,z_fb] = libdyn_new_feedback(sim);
    
    [sim, sum_] = ld_sum(sim, ev, list(u__, z_fb), 1, 1);
    [sim, tmp] = ld_ztf(sim, ev, sum_, 1/z);
    [sim, y] = ld_sat(sim, ev, tmp, min__, max__);
    
    [sim] = libdyn_close_loop(sim, y, z_fb);    
endfunction


function [sim, u] = ld_lin_awup_controller(sim, ev, r, y, Ta, tfR, min__, max__) // PARSEDOCU_BLOCK
// linear controller with anti reset windup implemented by bounding the integral state:
// e = r-y
// u = ld_limited_integrator( e, min__, max__ ) + tfR*e
    [sim, e] = ld_sum(sim, ev, list(r, y), 1, -1);
    
    [sim,u1] = ld_limited_integrator(sim, ev, e, min__, max__, Ta);
    [sim,u2] = ld_ztf(sim, ev, e, tfR);
    
    [sim,u] = ld_sum(sim, ev, list(u1,u2), 1,1);
endfunction


function [sim] = ld_print_angle(sim, ev, alpha, text) // PARSEDOCU_BLOCK
// Convert an angle in rad to degree and print to console
// 
// alpha - angle signal
// text - string
// 
    [sim, alpha_deg] = ld_gain(sim, ev, alpha, 1/%pi*180);
    [sim] = ld_printf(sim, ev, alpha_deg, text, 1);
endfunction

function [sim,pwm] = ld_pwm(sim, ev, plen, u) // PARSEDOCU_BLOCK
// 
// PWM generator
// 
// plen - period length
// u - modulation signal; Values are between 0 and 1.
//

    [sim,u] = ld_gain(sim, ev, u, plen);
    
    [sim,one] = ld_const(sim, ev, 1);
    
    [sim,modcount] = ld_modcounter(sim, ev, in=one, initial_count=0, mod=plen);
    
    [sim, test] = ld_add(sim, ev, list(modcount, u), [-1,1] );
    [sim,pwm] = ld_compare_01(sim, ev, test,  thr=0);
endfunction





