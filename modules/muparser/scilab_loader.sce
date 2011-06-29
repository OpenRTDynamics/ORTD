//
//    Copyright (C) 2010, 2011  Christian Klauer
//
//    This file is part of OpenRTDynamics, the Real Time Dynamic Toolbox
//
//    OpenRTDynamics is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    OpenRTDynamics is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with OpenRTDynamics.  If not, see <http://www.gnu.org/licenses/>.
//





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


//
// Use: http://help.scilab.org/docs/5.3.2/en_US/strsubst.html
//


//parNames = ['par1', 'par2']; par = [ 0.2. 0.4 ];
//inNames = ['u', 'e' ]; inlist = list(x,x);
//str = 'sin(par1) + par2';
//
function [sim,out] = ld_muparser_subst(sim, events, inlist, str, par, inNames, parNames)
  // 
  Nin = length(inlist);
  Nout = 1;
  
  inNames = inNames(:);
  parNames = parNames(:);
  
  [NinNames, tmp] = size(inNames);
  [NparNames, tmp] = size(parNames);
  
//  pause;

  if length(inlist) ~= NinNames then
      printf("inNames and inlist have to have equal length (%d != %d)", length(inlist), NinNames);
      error(".");
  end
  if length(par) ~= NparNames then
      error("parNames and par have to have equal length");
  end

  i = 1;
  while i <= Nin 
    if typeof(inNames(i)) ~= 'string' then
      error("ld_muparser2: Did not find a sting in invar");
    end


    if libdyn_is_ldobject(inlist(i)) == %F then
      error("mu_parser2: Did not find a libdyn object at some entry within inlist");
    end

    replacement = 'u' + string(i);
 //   printf("subst %s with %s\n", inNames(i), replacement);
    str = strsubst( str, inNames(i), replacement );

    i = i + 1;
  end
  

  i = 1;
  while i <= length(par) 
    if typeof(parNames(i)) ~= 'string' then
      error("ld_muparser2: Did not find a sting in parNames");
    end


    if typeof(par(i)) ~= 'constant' then
      error("mu_parser2: Did not find a skalar");
    end

    replacement = 'c' + string(i);
  //  printf("subst %s with %s\n", parNames(i), replacement);
    str = strsubst( str, parNames(i), replacement );


    i = i + 1;
  end


//  printf("The expression is now: %s\n", str);


  [sim,out] = ld_muparser(sim, events, inlist, str, par);
endfunction


// 
// 
// 
// 
// 
// function [sim,out] = ld_muparser2(sim, events, inlist, str, invar, par)
//   // 
//   Nin = length(inlist);
//   Nout = 1;
// 
//   parlist = new_irparam_set();
// 
// 
//   inlist = list();
// 
//   n_in = 0;
//   i = 1;
//   //i_ = 1:length(invar);
//   while i <= length(invar) 
//     if typeof(invar(i)) ~= 'string' then
//       error("ld_muparser2: Did not find a sting in invar");
//     end
// 
//     parlist = new_irparam_elemet_ivec(parlist, invar(i), 100+n_in); // save name of input variable
// 
//     n_in = n_in + 1;
//     i = i + 1;
// 
// 
//     if libdyn_is_ldobject(invar(i)) == %F then
//       error("mu_parser2: Did not find a libdyn object in invar");
//     end
// 
//     inlist(n_in) = invar(i);
// 
//     i = i + 1;
// 
//   end
//   
// 
//   pnr = 0;
//   i = 1;
//   //i_ = 1:length(invar);
//   while i <= length(par) 
//     if typeof(par(i)) ~= 'string' then
//       error("ld_muparser2: Did not find a sting in invar");
//     end
// 
//     parlist = new_irparam_elemet_ivec(parlist, ascii(par(i)), 10000+pnr); // save name of parameter
// 
//     pnr = pnr + 1;
//     i = i + 1;
// 
// 
//     if  typeof(par(i)) ~= 'constant' then
//       error("mu_parser2: Did not find a skalar");
//     end
// 
//     parlist = new_irparam_elemet_rvec(parlist, par(i), 20000+pnr); // save parameter value
//     
// 
//     i = i + 1;
// 
//   end
// 
// 
//   // store expression
//   str = ascii(str);
//   parlist = new_irparam_elemet_ivec(parlist, str, 10); 
// 
//   // combine ir parameters
//   blockparam = combine_irparam(parlist);
// 
// 
//   // block parameters
//   ipar = [Nin, Nout, blockparam.ipar];
//   rpar = [blockparam.rpar];
// 
//   btype = 11002; // block type id
//   
//   [sim,bid] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar);
// 
// 
//   [sim,blk] = libdyn_conn_equation(sim, blk, inlist); // connect all imputs
//   [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
// endfunction
// 
// 
