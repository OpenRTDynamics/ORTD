

// Interfacing functions are placed in this place


function [sim,bid] = libdyn_new_muparser(sim, events, Nin, Nout, str, float_param)
  btype = 11001;
  str = ascii(str);
  nparam = length(float_param);
  
  ipar = [Nin, Nout, nparam, length(str), str(:)'];
  rpar = [float_param(:)];
  
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar);
endfunction

function [sim,out] = ld_muparser(sim, events, inlist, str, float_param)
  // 
  Nin = length(inlist);
  Nout = 1;

  [sim,blk] = libdyn_new_muparser(sim, events, Nin, Nout, str, float_param);
  [sim,blk] = libdyn_conn_equation(sim, blk, inlist);
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction



function [sim,out] = ld_muparser2(sim, events, inlist, str, invar, par)
  // 
  Nin = length(inlist);
  Nout = 1;

  parlist = new_irparam_set();


  inlist = list();

  n_in = 0;
  i = 1;
  //i_ = 1:length(invar);
  while i <= length(invar) 
    if typeof(invar(i)) ~= 'string' then
      error("ld_muparser2: Did not find a sting in invar");
    end

    parlist = new_irparam_elemet_ivec(parlist, invar(i), 100+n_in); // save name of input variable

    n_in = n_in + 1;
    i = i + 1;


    if libdyn_is_ldobject(invar(i)) == %F then
      error("mu_parser2: Did not find a libdyn object in invar");
    end

    inlist(n_in) = invar(i);

    i = i + 1;

  end
  

  pnr = 0;
  i = 1;
  //i_ = 1:length(invar);
  while i <= length(par) 
    if typeof(par(i)) ~= 'string' then
      error("ld_muparser2: Did not find a sting in invar");
    end

    parlist = new_irparam_elemet_ivec(parlist, ascii(par(i)), 10000+pnr); // save name of parameter

    pnr = pnr + 1;
    i = i + 1;


    if  typeof(par(i)) ~= 'constant' then
      error("mu_parser2: Did not find a skalar");
    end

    parlist = new_irparam_elemet_rvec(parlist, par(i), 20000+pnr); // save parameter value
    

    i = i + 1;

  end


  // store expression
  str = ascii(str);
  parlist = new_irparam_elemet_ivec(parlist, str, 10); 

  // combine ir parameters
  blockparam = combine_irparam(parlist);


  // block parameters
  ipar = [Nin, Nout, blockparam.ipar];
  rpar = [blockparam.rpar];

  btype = 11002; // block type id
  
  [sim,bid] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar);


  [sim,blk] = libdyn_conn_equation(sim, blk, inlist); // connect all imputs
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


