

// Interfacing functions are placed in this place


function [sim]=ld_savefile(sim, events, fname, source, vlen) // PARSEDOCU_BLOCK
// Quick and easy dumping of signals to files
// 
// fname - string of the filename
// source *+ - source signal
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


function [sim, out] = ld_switch2to1(sim, events, cntrl, in1, in2) // PARSEDOCU_BLOCK
//
// A 2 to 1 switching Block
//
// cntr * - control input
// in1 *
// in2 *
// out * - output
//
// if cntrl > 0 : out = in1
// if cntrl < 0 : out = in2
//
  btype = 60001;
  [sim,blk] = libdyn_new_block(sim, events, btype, [], [], ...
                   insizes=[1, 1, 1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], ...
                   outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(cntrl, in1, in2) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction





function [sim, outlist] = ld_demux(sim, events, vecsize, invec) // PARSEDOCU_BLOCK
//
// Demultiplexer
//
// invec * - input vector signal to be split up
// outlist *LIST - list() of output signals
//
//
// Splits the input vector signal "invec" of size "vecsize" up into 
//
// outlist(1)
// outlist(2)
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



function [sim, out] = ld_mux(sim, events, vecsize, inlist) // PARSEDOCU_BLOCK
//    
// Multiplexer
//
// inlist *LIST - list() of input signals of size 1
// out *+ - output vector signal
// 
// 
// combines inlist(1), inlist(2), ...    
// to a vector signal "out" of size "vecsize", whereby each inlist(i) is of size 1
//    
  btype = 60001 + 2;	
  ipar = [vecsize; 0]; rpar = [];

  if (length(inlist) ~= vecsize) then
    printf("Incorect number of input ports to ld_mux. %d != %d\n", length(inlist), vecsize );
    error(".");
  end

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[ones(1,vecsize)], outsizes=[vecsize], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT*ones(1,vecsize) ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, inlist );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_hysteresis(sim, events, in, switch_on_level, switch_off_level, initial_state, onout, offout) // PARSEDOCU_BLOCK
// hysteresis - block
//
// in * - input
// out * -output
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


function [sim, out] = ld_modcounter(sim, events, in, initial_count, mod) // PARSEDOCU_BLOCK
// Modulo Counter - Block
//
// in * - input
// out * -output
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


function [sim, out] = ld_jumper(sim, events, in, steps) // PARSEDOCU_BLOCK
// jumper - block
//
// out *+ - vector of size steps
// in * - switching input
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


function [sim, out] = ld_memory(sim, events, in, rememberin, initial_state) // PARSEDOCU_BLOCK
// memory - block
//
// in * - input
// rememberin * - 
// out * - output
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


function [sim, out] = ld_abs(sim, events, in) // PARSEDOCU_BLOCK
// abs - block
//
// in * - input
// out * - output
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


function [sim, out] = ld_extract_element(sim, events, invec, pointer, vecsize ) // PARSEDOCU_BLOCK
  //
  // Extract one element of a vector
  //
  // invec *+ - the input vector signal
  // pointer * - the index signal
  // vecsize - length of input vector
  // 
  // out = invec[pointer], the first element is at pointer = 1
  //

  btype = 60001 + 8;	
  ipar = [ vecsize, ORTD.DATATYPE_FLOAT ]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[vecsize, 1], outsizes=[1], ...
                       intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

//   [sim,blk] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar               );


  [sim,blk] = libdyn_conn_equation(sim, blk, list(invec, pointer) );

  // connect each outport
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // ith port
endfunction


function [sim, out] = ld_constvec(sim, events, vec) // PARSEDOCU_BLOCK
// 
// a constant vector
// 
// vec *+ - the vector
// 
  btype = 60001 + 9;	
  ipar = [length(vec); 0]; rpar = [vec];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                   insizes=[1], outsizes=[ length(vec) ], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );
 
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_counter(sim, events, count, reset, resetto, initial) // PARSEDOCU_BLOCK
// 
// A resetable counter block
//
// count * - signal
// reset * - signal
// resetto * - signal
// initial - constant
// out * - output
// 
// increases out by count (out = out + count)
// 
// if reset > 0.5 then
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

function [sim, out] = ld_shift_register(sim, events, in, len) // FIXME TODO
// A shift register with access to the stored values
//
// in *
// out *+(len)
//    
// 
// 
// 
  btype = 60001 + 11;
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[ 1 ], outsizes=[len], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_lookup(sim, events, u, lower_b, upper_b, table, interpolation) // PARSEDOCU_BLOCK
// Lookup table - block
//
// in * - input
// out * - output
// 
// 
// lower_b - smallest value of the input signal to map to the table
// upper_b - biggest value of the input signal to map to the table
// table - the table (Scilab vector)
// 
// Mapping is done in a linear way:
//   out = table( (in - lowerin) / (upperin - lowerin) )
// 
// interpolation = 0 : no interpolation
// interpolation = 1 : linear interpolation
// 
// 

  btype = 60001 + 12;
  [sim,blk] = libdyn_new_block(sim, events, btype, [length(table), interpolation ], [ lower_b, upper_b, table(:)' ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(u) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_not(sim, events, in) // PARSEDOCU_BLOCK
// logic negation - block
//
// in * - input
// out * - output
// 
// out = 0, if in > 0.5  OR  out = 1, if in < 0.5
// 

  btype = 60001 + 13;
  [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_or(sim, events, inlist) // PARSEDOCU_BLOCK
// logic or - block
//
// in *LIST - list() of inputs (for now the exactly two inputs are possible)
// out * - output
// 
// 
// 
  Nin=length(inlist);

  if (Nin ~= 2) then
    error("invalid number of inputs");
  end

  insizes=ones(1, Nin);
  intypes=ones(1, Nin) * ORTD.DATATYPE_FLOAT;

  btype = 60001 + 14;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[  ], rpar=[   ], ...
                   insizes, outsizes=[1], ...
                   intypes, outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_iszero(sim, events, in, eps) // PARSEDOCU_BLOCK
//
// check if input is near zero
//
// in * - input
// out * - output
// 
// out = 1, if in between -eps and eps, othwewise out = 0
// 

  btype = 60001 + 15;
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[  ], rpar=[ eps ], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, out] = ld_limitedcounter(sim, events, count, reset, resetto, initial, lower_b, upper_b) // PARSEDOCU_BLOCK
// 
// A resetable, limited counter block
//
// count * - signal
// reset * - signal
// resetto * - signal
// initial - constant
// out * - output
// 
// increases out by count (out = out + count), but count is always between lower_b and upper_b
// 
// if reset > 0.5 then
//   out = resetto
//
// initially out is set to initial
// 
// 

  if (lower_b > upper_b) then
    error("lower_b is greater than upper_b");
  end

  btype = 60001 + 16;
  ipar = [  ]; rpar = [ initial, lower_b, upper_b ];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                       insizes=[1,1,1], outsizes=[1], ...
                       intypes=[ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT], ...
                       outtypes=[ORTD.DATATYPE_FLOAT]  );

  [sim,blk] = libdyn_conn_equation(sim, blk, list( count, reset, resetto ) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

// function [sim, out] = ld_resetable_int(sim, events, in) // PARSEDOCU_BLOCK
// // logic negation - block
// //
// // in * - input
// // out * - output
// // 
// // out = 0, if in > 0.5  OR  out = 1, if in < 0.5
// // 
// 
//   btype = 60001 + 13;
//   [sim,blk] = libdyn_new_block(sim, events, btype, [  ], [  ], ...
//                    insizes=[1], outsizes=[1], ...
//                    intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );
// 
//   [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
//   [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
// endfunction


// 
//  Vector functions
// 


function [sim, out] = ld_vector_diff(sim, events, in, vecsize) // PARSEDOCU_BLOCK
//    
// Vector differentiation with respect to the index
// 
// in *+(vecsize) - vector signal of size "vecsize"
// out *+(vecsize-1) - vector signal of size "vecsize-1"
//
// Equivalent to Scilab 'diff' function
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

function [sim, index] = ld_vector_findthr(sim, events, in, thr, greater, vecsize) // PARSEDOCU_BLOCK
//    
// in *+(vecsize) - input
// thr * - threshold signal
// index * - output
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

function [sim, out] = ld_vector_abs(sim, events, in, vecsize) // PARSEDOCU_BLOCK
// Vector abs()
//
// in *+(vecsize) - input
// out *+(vecsize) - output
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

function [sim, out] = ld_vector_gain(sim, events, in, gain, vecsize) // PARSEDOCU_BLOCK   
// Vector gain
//
// in *+(vecsize) - input
// out *+(vecsize) - output
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

function [sim, out] = ld_vector_extract(sim, events, in, from, window_len, vecsize) // PARSEDOCU_BLOCK
//    
// Extract "in" from to from+window_len
// 
//  in *+(vecsize) - vector signal
//  from * - index signal
//  out *+(window_len) - output signal
//
//    
  btype = 60001 + 54;	
  ipar = [vecsize; window_len]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize, 1], outsizes=[window_len], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT, ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
//   disp('new vextr\n');
//   disp( [vecsize, 1] );


  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in, from ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction

function [sim, index] = ld_vector_minmax(sim, events, in, findmax, vecsize) // PARSEDOCU_BLOCK
// Min / Max of a vector
//
// in *+(vecsize) 
// index * - the index starting at 1,, where the max / min was found
//    
  btype = 60001 + 55;	
  ipar = [vecsize; findmax]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize], outsizes=[1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in ) );

  [sim,index] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


function [sim, out] = ld_vector_addscalar(sim, events, in, add, vecsize) // PARSEDOCU_BLOCK
// add "add" to the vector
// 
//  add * - signal
//  in *+(vecsize) - vector signal
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

function [sim, out] = ld_vector_sum(sim, events, in, vecsize) // PARSEDOCU_BLOCK
// sum over "in"
//
// in *+(vecsize)
// out *
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

function [sim, out] = ld_vector_addsum(sim, events, in1, in2, vecsize) // FIXME TODO 
// multiplicate two vectors and calc the sum of the result ( sum( in1 .* in2) )
//
// in1 *+(vecsize)
// in2 *+(vecsize)
// out *
//    
  btype = 60001 + 58;
  ipar = [vecsize]; rpar = [];

  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                                     insizes=[vecsize,vecsize ], outsizes=[1], ...
                                     intypes=[ ORTD.DATATYPE_FLOAT,ORTD.DATATYPE_FLOAT ], outtypes=[ORTD.DATATYPE_FLOAT] );
 
  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list( in1, in2 ) );

  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



// 
// 
// 
// 







//
// Macros
//
//


function [sim, y] = ld_mute( sim, ev, u, cntrl, mutewhengreaterzero ) // PARSEDOCU_BLOCK
//    
//    Mute a signal based on cntrl-signal
//
//    ev - event
//    u * - input
//    y * - output
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
// 
// u * - input
// y * - output
// 
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
// alpha * - angle signal
// text - string
// 
    [sim, alpha_deg] = ld_gain(sim, ev, alpha, 1/%pi*180);
    [sim] = ld_printf(sim, ev, alpha_deg, text, 1);
endfunction

function [sim, pwm] = ld_pwm(sim, ev, plen, u) // PARSEDOCU_BLOCK
// 
// PWM generator
// 
// plen - period length
// u * - modulation signal; Values are between 0 and 1.
// pwm * - pwm output
//

    [sim,u] = ld_gain(sim, ev, u, plen);
    
    [sim,one] = ld_const(sim, ev, 1);
    
    [sim,modcount] = ld_modcounter(sim, ev, in=one, initial_count=0, mod=plen);
    
    [sim, test] = ld_add(sim, ev, list(modcount, u), [-1,1] );
    [sim,pwm] = ld_compare_01(sim, ev, test,  thr=0);
endfunction


function [sim,y] = ld_alternate( sim, ev, start_with_zero ) // PARSEDOCU_BLOCK
//
// generate an alternating sequence     
//
// y * - output
//
//
// [0, 1, 0, 1, 0, ... ], if start_with_zero == %T
// [1, 0, 1, 0, 1, ... ], if start_with_zero == %F
//
  z = poly(0,'z');    

  [sim,one] = ld_const(sim, ev, 1);

  [sim, fb] = libdyn_new_feedback();
  
  [sim, su ] = ld_add(sim, ev, list(fb, one), [-1,1] );
  [sim, del] = ld_ztf(sim, ev, su, 1/z); // a delay of one sample
  
  [sim] = libdyn_close_loop(sim, del, fb);
  
  y = del; 
  
  if (start_with_zero == %F) then
    [sim, y] = ld_not(sim, ev, y);
  end
    
endfunction


// 
// Blocks, which C functions have not been move to the basic module yet, but the interfacing function
// 



// compare block. If input > thr: 
// optional_cmp_mode342 == 0: output = 1; else -1
// optional_cmp_mode342 == 1: output = 1; else 0
function [sim,bid] = libdyn_new_compare(sim, events, thr, optional_cmp_mode342)    
  if (exists('optional_cmp_mode342') ~= 1) then
    optional_cmp_mode342 = 0;
  end

  btype = 140;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [optional_cmp_mode342], [thr]);
endfunction


function [sim,y] = ld_compare(sim, events, in,  thr) // PARSEDOCU_BLOCK
//
// compare block. 
//   thr - constant
//   in * - signal
//   y *
// If in > thr: y = 1; else y = -1
//

    [sim,blk] = libdyn_new_compare(sim, events, thr);
    [sim,blk] = libdyn_conn_equation(sim, blk, list(in,0));
    [sim,y] = libdyn_new_oport_hint(sim, blk, 0);    
endfunction

function [sim,y] = ld_compare_01(sim, events, in,  thr) // PARSEDOCU_BLOCK
//
// compare block. 
//   thr - constant
//   in - signal
//   y *
// If in > thr: y = 1; else y = 0
//

    [sim,blk] = libdyn_new_compare(sim, events, thr, 1); // mode = 1
    [sim,blk] = libdyn_conn_equation(sim, blk, list(in,0));
    [sim,y] = libdyn_new_oport_hint(sim, blk, 0);    
endfunction





















//
// Blocks
//


// FIXME: komische Funktion (noch gebraucht?)
function [sim,bid] = libdyn_new_blk_sum_pn(sim)
  btype = 10;
  [sim,oid] = libdyn_new_objectid(sim);
  id = oid; // id for this parameter set
  
  sim.parlist = new_irparam_elemet(sim.parlist, id, IRPAR_LIBDYN_BLOCK, [btype; oid], []);
endfunction


function [sim,bid] = libdyn_new_blk_gen(sim, events, symbol_name, ipar, rpar)
  btype = 5000;
  str = str2code(symbol_name);
  
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [length(str); str; ipar], [rpar]);
endfunction


function [sim,bid] = libdyn_new_blk_sum(sim, events, c1, c2)
  btype = 12;
  
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [], [c1; c2]);
  
//  [sim,bid] = libdyn_new_blockid(sim);
//  id = bid; // id for this parameter set
//  
//  sim.parlist = new_irparam_elemet(sim.parlist, id, IRPAR_LIBDYN_BLOCK, [btype; bid], [c1; c2]);
endfunction

function [sim,bid] = libdyn_new_blk_zTF(sim, events, H)
  btype = 30;
  bip = [ degree(H.num); degree(H.den) ];
  brp = [ coeff(H.num)'; coeff(H.den)' ];

  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [bip], [brp]);
  
//  [sim,bid] = libdyn_new_blockid(sim);
//  id = bid; // id for this parameter set
//  
//  
//  sim.parlist = new_irparam_elemet(sim.parlist, id, IRPAR_LIBDYN_BLOCK, [btype; bid; bip], [brp]);
endfunction

function [sim,bid] = libdyn_new_blk_const(sim, events, c1)
  btype = 40;
  [sim,bid] = libdyn_new_block(sim, events, btype, [], [c1], ...
                   insizes=[], outsizes=[1], ...
                   intypes=[], outtypes=[ORTD.DATATYPE_FLOAT]  );
endfunction

function [sim,bid] = libdyn_new_blk_gain(sim, events, c)
  btype = 20;
  [sim,bid] = libdyn_new_block(sim, events, btype, [], [c], ...
                   insizes=[1], outsizes=[1], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );
endfunction


function [sim,bid] = libdyn_new_blk_sat(sim, events, lowerlimit, upperlimit)
  btype = 50;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [], [lowerlimit, upperlimit]);
endfunction


//
// A switching Block
// inputs = [control_in, signal_in]
// if control_in > 0 : signal_in is directed to output 1; output_2 is set to zero
// if control_in < 0 : signal_in is directed to output 2; output_1 is set to zero
//
function [sim,bid] = libdyn_new_blk_switch(sim, events)
  btype = 60;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [], []);
endfunction

// Multiplication 
// d1, d2: multiplicate (=0) or divide (=1) corresponding input; need exactly 2 inputs
function [sim,bid] = libdyn_new_blk_mul(sim, events, d1, d2)
  btype = 70;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [d1, d2], []);
endfunction

// function generator
// at the moment only sinus (shape = ?)
// input port 1: amplitude
// input port 2: frequency
function [sim,bid] = libdyn_new_blk_fngen(sim, events, shape_)
  btype = 80;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [shape_], []);
endfunction
 
// serial to parallel
function [sim,bid] = libdyn_new_blk_s2p(sim, events, len)
  btype = 90;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [len], []);
endfunction

//
// Sample play block
//
// plays the sequence stored in r
// each time event 0 occurs the next value of r is put out
// sampling start either imediadedly (initial_play=1) or on event 1.
// Event 2 stops sampling and set ouput to last values (mute_afterstop = 0 and hold_last_values == 1) 
// or zero (mute_afterstop = 1)
//
function [sim,bid] = libdyn_new_blk_play(sim, events, r, initial_play, hold_last_value, mute_afterstop)
  if (exists('initial_play') ~= 1) then
    initial_play = 1;  
  end
  if (exists('hold_last_value') ~= 1) then
    hold_last_value = 0;  
  end
  if (exists('mute_afterstop') ~= 1) then
    mute_afterstop = 0;  
  end
  
  btype = 100;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [length(r), initial_play, hold_last_value, mute_afterstop], [r]);
endfunction


//
// A swich with two inputs and one output depending on a state. initial state=initial_state
// state 0 means the first input is feed through
// state 1 means the first input is feed through
//
// Switching occurs to state 0 if event 1 occurs
// Switching occurs to state 1 if event 2 occurs
// event 0 is the normal regular event
//

function [sim,bid] = libdyn_new_blk_2to1evsw(sim, events, initial_state)
  btype = 110;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [initial_state], []);
endfunction

// Lookup table: Inputs between lowerin and upperin will be mapped linear to the indices of table
//               The corresponsing element of table will be the output
function [sim,bid] = libdyn_new_blk_lkup(sim, events, lowerin, upperin, table)
  btype = 120;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [length(table)], [ lowerin, upperin, table(:)' ]);
endfunction


// Dump at max maxlen samples to file "filename"; start automatically if autostart == 1
function [sim,bid] = libdyn_new_blk_filedump(sim, events, filename, vlen, maxlen, autostart)
  btype = 130;
  fname = ascii(filename);
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [maxlen, autostart, vlen, length(fname), fname(:)'], []);
endfunction


// delay of delay_len samples block
function [sim,bid] = libdyn_new_delay(sim, events, delay_len)
  btype = 150;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [delay_len], []);
endfunction

function [sim,bid] = libdyn_new_flipflop(sim, events, initial_state)
  btype = 160;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [initial_state], []);
endfunction





function [sim,bid] = libdyn_new_interface(sim, events, len)
  btype = 4000;
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [len], []);
endfunction






//////////////////////////////////////////////////////////
// fancy shortcuts, that *should* be used by the user
//////////////////////////////////////////////////////////

function [sim,c] = ld_const(sim, events, val) // PARSEDOCU_BLOCK
//
// A constant val
//

    [sim,c] = libdyn_new_blk_const(sim, events, val); // Instead of event a predefined initial event that only occurs once should be used
    [sim,c] = libdyn_new_oport_hint(sim, c, 0);    
endfunction

function [sim,sum_] = ld_sum(sim, events, inp_list, fak1, fak2) 
// FIXME obsolete
    [sim,sum_] = libdyn_new_blk_sum(sim, events, fak1, fak2);
    [sim,sum_] = libdyn_conn_equation(sim, sum_, inp_list);  
    [sim,sum_] = libdyn_new_oport_hint(sim, sum_, 0);    
endfunction

function [sim,sum_] = ld_add(sim, events, inp_list, fak_list) // PARSEDOCU_BLOCK
//
// Add signals (linear combination)
// inp_list = list( in1, in2 )  ; fak_list = [ c1, c2 ]
// sum_ = in1 * c1 + in2 * c2
//

    [sim,sum_] = libdyn_new_blk_sum(sim, events, fak_list(1), fak_list(2));
    [sim,sum_] = libdyn_conn_equation(sim, sum_, inp_list);  
    [sim,sum_] = libdyn_new_oport_hint(sim, sum_, 0);    
endfunction


function [sim,mul_] = ld_mul(sim, events, inp_list, muldiv1, muldiv2)
// Multiplication 
// muldiv1/2: multiplicate (=0) or divide (=1) corresponding input; need exactly 2 inputs
 // FIXME obsolete
    [sim,mul_] = libdyn_new_blk_mul(sim, events, muldiv1, muldiv2);
    [sim,mul_] = libdyn_conn_equation(sim, mul_, inp_list);  
    [sim,mul_] = libdyn_new_oport_hint(sim, mul_, 0);    
endfunction

function [sim,mul_] = ld_dot(sim, events, inp_list, muldiv1_list)
// Multiplication 
// muldiv1/2: multiplicate (=0) or divide (=1) corresponding input; need exactly 2 inputs
// inp_list = list( in1, in2 )  ; muldiv1_list = [ muldiv1, muldiv2 ]
    [sim,mul_] = libdyn_new_blk_mul(sim, events, muldiv1_list(1), muldiv1_list(2) );
    [sim,mul_] = libdyn_conn_equation(sim, mul_, inp_list);  
    [sim,mul_] = libdyn_new_oport_hint(sim, mul_, 0);    
endfunction

function [sim,mul_] = ld_mult(sim, events, inp_list, muldiv1_list) // PARSEDOCU_BLOCK
//
// Multiplication 
// muldiv1/2: multiplicate (=0) or divide (=1) corresponding input; need exactly 2 inputs
// inp_list = list( in1, in2 )  ; muldiv1_list = [ muldiv1, muldiv2 ]
//
    [sim,mul_] = libdyn_new_blk_mul(sim, events, muldiv1_list(1), muldiv1_list(2) );
    [sim,mul_] = libdyn_conn_equation(sim, mul_, inp_list);  
    [sim,mul_] = libdyn_new_oport_hint(sim, mul_, 0);    
endfunction


function [sim,gain] = ld_gain(sim, events, inp_list, gain) // PARSEDOCU_BLOCK
//
// A simple gain
//
//
  [inp] = libdyn_extrakt_obj( inp_list ); // compatibility

  [sim,gain] = libdyn_new_blk_gain(sim, events, gain);  
  [sim,gain] = libdyn_conn_equation(sim, gain, list(inp,0));
  [sim,gain] = libdyn_new_oport_hint(sim, gain, 0);    
endfunction

function [sim,sign_] = ld_sign(sim, events, inp_list, thr) // PARSEDOCU_BLOCK
//
// return the sign of the input sigal
// either 1 or -1
//
    [inp] = libdyn_extrakt_obj( inp_list ); // compatibility

    [sim,sign_] = libdyn_new_compare(sim, events, thr);
    [sim,sign_] = libdyn_conn_equation(sim, sign_, list(inp));
    [sim,sign_] = libdyn_new_oport_hint(sim, sign_, 0);
endfunction

function [sim,lkup] = ld_lkup(sim, events, inp_list, lower_b, upper_b, table)
// lookup table
//
// inp_list - input signal
// table - a vector of values to look up
// the input is mapped between lower_b and upper_b to the
// index within table
  [inp] = libdyn_extrakt_obj( inp_list ); // compatibility

    [sim,lkup] = libdyn_new_blk_lkup(sim, events, lower_b, upper_b, table);
    [sim,lkup] = libdyn_conn_equation(sim, lkup, list(inp));
    [sim,lkup] = libdyn_new_oport_hint(sim, lkup, 0);
endfunction
  
function [sim,fngen] = ld_fngen(sim, events, inp_list, shape_)
// function generator
// need cleanup
    [sim,fngen] = libdyn_new_blk_fngen(sim, events, shape_)
    [sim,fngen] = libdyn_conn_equation(sim, fngen, inp_list);
    [sim,fngen] = libdyn_new_oport_hint(sim, fngen, 0);
endfunction


function [sim,delay] = ld_delay(sim, events, inp_list, delay_len)
    [inp] = libdyn_extrakt_obj( inp_list ); // compatibility

    [sim,delay] = libdyn_new_delay(sim, events, delay_len)
    [sim,delay] = libdyn_conn_equation(sim, delay, list(inp));
    [sim,delay] = libdyn_new_oport_hint(sim, delay, 0);    
endfunction

function [sim,y] = ld_ztf(sim, events, inp_list, H) // PARSEDOCU_BLOCK
//
// Time discrete transfer function
// H is give as a Scilab rational
//

  [inp] = libdyn_extrakt_obj( inp_list ); // compatibility

    [sim,tf] = libdyn_new_blk_zTF(sim, events, H);
    [sim,y] = libdyn_conn_equation(sim, tf, list(inp));
    [sim,y] = libdyn_new_oport_hint(sim, y, 0);    
endfunction

function [sim,y] = ld_sat(sim, events, inp_list, lowerlimit, upperlimit) // PARSEDOCU_BLOCK
//
// Saturation between lowerlimit and upperlimit
//


  [inp] = libdyn_extrakt_obj( inp_list ); // compatibility

    [sim,sat] = libdyn_new_blk_sat(sim, events, lowerlimit, upperlimit);
    [sim,y] = libdyn_conn_equation(sim, sat, list(inp));
    [sim,y] = libdyn_new_oport_hint(sim, y, 0);    
endfunction

function [sim,y] = ld_flipflop(sim, events, set0, set1, reset, initial_state) // PARSEDOCU_BLOCK
//
// A flip-flop (FIXME: more documentation needed)
//
// set0, set1, reset * - control of the flipflop
// initial_state - constant
//
    [sim,blk] = libdyn_new_flipflop(sim, events, initial_state);
    [sim,blk] = libdyn_conn_equation(sim, blk, list(set0,0, set1,0, reset,0)); // FIXME: remove ,0
    [sim,y] = libdyn_new_oport_hint(sim, blk, 0);    
endfunction




function [sim,bid] = libdyn_new_printf(sim, events, str, insize)
  btype = 170;
  str = ascii(str);
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, [insize, length(str), str(:)'], []);
endfunction

function [sim] = ld_printf(sim, events, in, str, insize) // PARSEDOCU_BLOCK
//
// Print data to stdout (the console)
//
// in *+(insize) - vectorial input signal
//
// str is a string that is printed followed by the signal vector in
// of size insize
//
  [sim,blk] = libdyn_new_printf(sim, events, str, insize);
  [sim,blk] = libdyn_conn_equation(sim, blk, list(in,0) );
endfunction





function [sim,out_1, out_2] = ld_switch(sim, events, cntrl, in) // PARSEDOCU_BLOCK
//
// A switching Block
// inputs = [control_in, signal_in]
// if control_in > 0 : signal_in is directed to output 1; output_2 is set to zero
// if control_in < 0 : signal_in is directed to output 2; output_1 is set to zero
//
    [sim,blk] = libdyn_new_blk_switch(sim, events);
    [sim,blk] = libdyn_conn_equation(sim, blk, list(cntrl,0, in,0));
    
    [sim,out_1] = libdyn_new_oport_hint(sim, blk, 0);
    [sim,out_2] = libdyn_new_oport_hint(sim, blk, 1);
endfunction


function [sim,y] = ld_play_simple(sim, events, r) // PARSEDOCU_BLOCK
//
// Simple sample play block
//
// plays the sequence stored in r
// each time event 0 occurs the next value of r is put out
//


  [sim,y] = libdyn_new_blk_play(sim, events, r, 1, 1, 0);
  [sim,y] = libdyn_new_oport_hint(sim, y, 0);    
endfunction



function [sim,save_]=libdyn_dumptoiofile(sim, events, fname, source) //OBSOLET
// Quick and easy dumping of signals to files in one line of code
// obsolete version
  [source] = libdyn_extrakt_obj( source ); // compatibility

  // source: a list with a block + a port
  [sim,save_] = libdyn_new_blk_filedump(sim, events, fname, 1, 0, 1);
  [sim,save_] = libdyn_conn_equation(sim, save_, list(source, 0) );
endfunction


function [sim,save_]=ld_dumptoiofile(sim, events, fname, source) //OBSOLET
//
// Quick and easy dumping of signals to files
// source - signal of size 1 (at the moment)
// fname - filename string
//
  [inp] = libdyn_extrakt_obj( source ); // compatibility

  // source: a list with a block + a port
  [sim,save_] = libdyn_new_blk_filedump(sim, events, fname, 1, 0, 1);
  [sim,save_] = libdyn_conn_equation(sim, save_, list(source) );
endfunction




