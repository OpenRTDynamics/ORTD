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
