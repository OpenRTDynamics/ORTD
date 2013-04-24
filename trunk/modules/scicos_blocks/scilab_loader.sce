

// Interfacing functions are placed in this place





// An example could be the following scilab function. PARSEDOCU_BLOCK is a hint for the documentation generator to generate documentation for this block
function [sim, outlist] = ld_scicosblock(sim, events, inlist, cosblk) // PARSEDOCU_BLOCK
// 
// %PURPOSE: include a scicos block / schematic (EXPERIMENTAL)
//
// inlist  - list() of input ports forwarded to the Scicosblock
// outlist  - list() of output ports forwarded from the Scicosblock
// 
// cosblk - structure as loaded by ortd_getcosblk containg the block parameters 
//

  

  model = cosblk;

//   identstr = model.blockname;
  identstr = model.sim(1); // name of C-comp fn

  printf("Including Scicos Block %s\n", identstr);
  printf("Make sure that the C function %s() is included in the binary used for execution\n", identstr);

  
//    if ( length(cosblk.in) ~= 1 ) then
//      error("Only one input port to the Scicos-Block allowed at the moment");
//    end
//  
//    if ( length(cosblk.out) ~= 1 ) then
//      error("Only one output port to the Scicos-Block allowed at the moment");
//    end
 
 
   Nin = length(cosblk.in);
   Nout = length(cosblk.out);

  
//           if ( irpar_get_ivec(&insizes, ipar, rpar, 10) < 0 ) error = 1 ;
//         if ( irpar_get_ivec(&outsizes, ipar, rpar, 11) < 0 ) error = 1 ;
//         if ( irpar_get_ivec(&intypes, ipar, rpar, 12) < 0 ) error = 1 ;
//         if ( irpar_get_ivec(&outtypes, ipar, rpar, 13) < 0 ) error = 1 ;
//         if ( irpar_get_ivec(&param, ipar, rpar, 18) <  0 ) error = 1 ;
//         
//         int dfeed = param.v[1];
// 
//     
//     if ( irpar_get_ivec(&identstr__, ipar, rpar, 20) < 0 ) error = 1 ;
//     if ( irpar_get_ivec(&block_ipar, ipar, rpar, 21) < 0 ) error = 1 ;
//     if ( irpar_get_rvec(&block_rpar, ipar, rpar, 22) < 0 ) error = 1 ;
//     if ( irpar_get_rvec(&dstate, ipar, rpar, 23) < 0 ) error = 1 ;
//     
  

  dfeed = 1;
  parameters = [dfeed];
  
  parlist = new_irparam_set();

  parlist = new_irparam_elemet_ivec(parlist, cosblk.in, 10);
  parlist = new_irparam_elemet_ivec(parlist, cosblk.out, 11);
  parlist = new_irparam_elemet_ivec(parlist, ORTD.DATATYPE_FLOAT*ones(cosblk.in) , 12); // only float is supported by now
  parlist = new_irparam_elemet_ivec(parlist, ORTD.DATATYPE_FLOAT*ones(cosblk.out) , 13); // only float is supported by now
  parlist = new_irparam_elemet_ivec(parlist, parameters, 18);


  parlist = new_irparam_elemet_ivec(parlist, ascii(identstr), 20);
  parlist = new_irparam_elemet_ivec(parlist, model.ipar, 21);
  parlist = new_irparam_elemet_rvec(parlist, model.rpar, 22);
  parlist = new_irparam_elemet_rvec(parlist, model.dstate, 23);

  blockparam = combine_irparam(parlist);



 btype = 15200 + 0;; // the same id you are giving via the "libdyn_compfnlist_add" C-function
//  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ 0, Nin, Nout, length(identstr), ascii(identstr)  ], rpar=[  ], ...
//                   insizes=[Nin], outsizes=[Nout], ...
//                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );



 [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ blockparam.ipar(:)'  ], rpar=[ blockparam.rpar(:)' ], ...
                  insizes=[  cosblk.in(:)' ], outsizes=[ cosblk.out(:)' ], ...
                  intypes=[ones(1,Nin)*ORTD.DATATYPE_FLOAT], outtypes=[ones(1,Nout)*ORTD.DATATYPE_FLOAT]  );


                  
   [sim,blk] = libdyn_conn_equation(sim, blk, inlist );
 
   outlist = list();
  for i = 0:(Nout-1)
    [sim,out] = libdyn_new_oport_hint(sim, blk, i);   // ith port
    outlist(i+1) = out;
  end

 
 //[sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction




function cosblk=ortd_getcosblk2(blockname, flag, cachefile)  // PARSEDOCU_BLOCK
  //
  // %PURPOSE: Extract information from Scicos block interfacing function macros (*.sci) files
  //
  // pathtoscifile - The interfacing function macro (a *.sci file)
  // flag - one of 'defaults', 'rundialog'
  //


  //exec(blockname + '_c.sci');
//   exec(pathtoscifile);

//  Scicos
//
//  Copyright (C) INRIA - METALAU Project <scicos@inria.fr>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// See the file ../license.txt
//

function model=scicos_model(v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11,v12,v13,v14,v15,v16,...
                            v17,v18,v19,v20,v21,v22,v23,v24,v25,v26)

  //initialisation de model mlist
  if exists('sim','local')==0 then sim='',end
  if exists('in','local')==0 then in=[],end
  if exists('in2','local')==0 then in2=[],end
  if exists('intyp','local')==0 then intyp=1,end
  if exists('out','local')==0 then out=[],end
  if exists('out2','local')==0 then out2=[],end
  if exists('outtyp','local')==0 then outtyp=1,end
  if exists('evtin','local')==0 then evtin=[],end
  if exists('evtout','local')==0 then evtout=[],end
  if exists('state','local')==0 then state=[],end
  if exists('dstate','local')==0 then dstate=[],end
  if exists('odstate','local')==0 then odstate=list(),end
  if exists('opar','local')==0 then opar=list(),end
  if exists('rpar','local')==0 then rpar=[],end
  if exists('ipar','local')==0 then ipar=[],end
  if exists('blocktype','local')==0 then blocktype='c',end
  if exists('firing','local')==0 then firing=[],end
  if exists('dep_ut','local')==0 then dep_ut=[%f %f],end
  if exists('label','local')==0 then label='',end
  if exists('nzcross','local')==0 then nzcross=0,end
  if exists('nmode','local')==0 then nmode=0,end
  if exists('equations','local')==0 then equations=list(),end

//  model=mlist(['model','sim','in','in2','intyp','out','out2','outtyp',...
//               'evtin','evtout','state','dstate','odstate','rpar','ipar','opar',...
//               'blocktype','firing','dep_ut','label','nzcross',..
//               'nmode','equations'],..
//               sim,in,in2,intyp,out,out2,outtyp,evtin,evtout,state,dstate,odstate,..
//               rpar,ipar,opar,blocktype,firing,dep_ut,label,nzcross,nmode,equations)

			model.sim=sim,..
			model.in=in,..
			model.in2=in2,..
			model.intyp=intyp,..
			model.out=out,..
			model.out2=out2,..
			model.outtyp=outtyp,..
//			model.evtin=clkinput,..
//			model.evtout=clkoutput,..
			model.firing=firing,..
			model.state=state,..
			model.dstate=dstate,..
			model.odstate=odstate,..
			model.rpar=rpar,..
			model.ipar=ipar,..
			model.opar=opar,..
			model.blocktype='c',..
			model.dep_ut=dep_ut,..
			model.nzcross=nzcross,..
			model.nmode=nmode  
endfunction


  function [model,graphics,ok]=check_io(model,graphics,in,out,clkin,clkout,in_implicit,out_implicit)
    // no actual check, just a copy into the model structure
    ok=%t


    model.evtin=clkin
    model.evtout=clkout
    model.in = in;
    model.out = out;
    model.evkin = clkin;
    model.evout = clkout;
  endfunction


  function x=standard_define( a, b, c, d )
    x.a =  a;
    x.b = b; // model
    x.c = c(:); // exprs
    x.d = d;

    x.a =  a;
    x.model = b; // model
    x.graphics.exprs = c(:); // exprs
    x.d = d;
  endfunction
  
  if (flag == 'usecachefile') then
      [lhs,rhs]=argn(0);
      if rhs >= 3 then
        try
          load(cachefile);  // ideally results in a new variable X, which is a structure
          printf("Using cachefile %s\n", cachefile);
          
          cosblk = X.model;
          cosblk.timestamp = getdate();
          cosblk.blockname = blockname;

        catch
          1;
        end
        1;
      else
        error("No cachefile was specified");
      end

      
  end
  
  if (flag == 'rundialog') then
  
    // overwrite a potenially already available X, which does not belong this code.
    X=0; // make typeof X to be constant
  
    // check for cached cosblk
      [lhs,rhs]=argn(0);
      if rhs >= 3 then
        try
          load(cachefile);  // ideally results in a new variable X, which is a structure
          printf("Using cachefile %s\n", cachefile);
        catch
          1;
        end
        1;
      end
      
    // if not available, call standard_define
    if (typeof(X) == 'constant') then // no new X was loaded, call macro for default values
      printf("loading block''s standard parameters\n");
    
      definecommand = "" + blockname + "(job=''define'',arg1=0,arg2=0);";
      X = eval(definecommand);
//       cosblk = X.b;
    end

    

//     arg1.graphics.exprs = X.graphics.exprs;
    arg1.graphics = X.graphics;
    arg1.model = X.model;
    definecommand = "" + blockname + "(job=''set'',arg1,arg2=0);";
    X = eval(definecommand);

    
    cosblk = X.model;
    cosblk.timestamp = getdate();
    cosblk.blockname = blockname;
    
    // print info
    printf("I/O of Scicosblock:\n  insizes=%s,\n  outsizes=%s\n", sci2exp( cosblk.in(:)' ), sci2exp( cosblk.out(:)' ) );

    // check for cached cosblk
    [lhs,rhs]=argn(0) 
    if rhs >= 3 then
      printf("Saving cachefile %s\n", cachefile);
      save(cachefile, X);
    end
      

  end
//  clear X;

endfunction




function cosblk=ortd_getcosblk(blockname, pathtoscifile)  // PARSEDOCU_BLOCK
  //
  // %PURPOSE: Extract information from Scicos block interfacing function macros (*.sci) files
  //
  // pathtoscifile - The interfacing function macro (a *.sci file)
  //

  
  // check if pathtoscifile is a function

  //exec(blockname + '_c.sci');
//   if (typeof(pathtoscifile) == 'string') then
   if pathtoscifile ~= "" then
     exec(pathtoscifile);
   end
//   end

  function model = scicos_model(sim,..
			in,..
			in2,..
			intyp,..
			out,..
			out2,..
			outtyp,..
			evtin,..
			evtout,..
			firing,..
			state,..
			dstate,..
			odstate,..
			rpar,..
			ipar,..
			opar,..
			blocktype,..
			dep_ut,..
			nzcross,..
			nmode )
			
			model.sim=sim,..
			model.in=in,..
			model.in2=in2,..
			model.intyp=intyp,..
			model.out=out,..
			model.out2=out2,..
			model.outtyp=outtyp,..
			model.evtin=clkinput,..
			model.evtout=clkoutput,..
			model.firing=firing,..
			model.state=x,..
			model.dstate=Z,..
			model.odstate=odstate,..
			model.rpar=rpar,..
			model.ipar=ipar,..
			model.opar=opar,..
			model.blocktype='c',..
			model.dep_ut=dep_ut,..
			model.nzcross=nzcross,..
			model.nmode=nmode           
			
  endfunction

  function x=standard_define( a, b, c, d )
    x.a =  a;
    x.b = b;
    x.c = c;
    x.d = d;
  endfunction

  //definecommand = "[x,y,typ]=" + blockname + "_c(job=''define'',arg1=0,arg2=0);";
  definecommand = "" + blockname + "_c(job=''define'',arg1=0,arg2=0);";
  X = eval(definecommand);
  cosblk = X.b;
  cosblk.timestamp = getdate();
  cosblk.blockname = blockname;
  clear X;

endfunction

// function cosblk=ortd_getcosblk2(blockname, pathtoscifile)  // PARSEDOCU_BLOCK
//   //
//   // %PURPOSE: Extract information from Scicos block interfacing function macros (*.sci) files
//   //
//   // pathtoscifile - The interfacing function macro (a *.sci file)
//   //
// 
// 
//   //exec(blockname + '_c.sci');
//   exec(pathtoscifile);
// 
//   function model = scicos_model(sim,..
// 			in,..
// 			in2,..
// 			intyp,..
// 			out,..
// 			out2,..
// 			outtyp,..
// 			evtin,..
// 			evtout,..
// 			firing,..
// 			state,..
// 			dstate,..
// 			odstate,..
// 			rpar,..
// 			ipar,..
// 			opar,..
// 			blocktype,..
// 			dep_ut,..
// 			nzcross,..
// 			nmode )
// 			
// 			model.sim=sim,..
// 			model.in=in,..
// 			model.in2=in2,..
// 			model.intyp=intyp,..
// 			model.out=out,..
// 			model.out2=out2,..
// 			model.outtyp=outtyp,..
// 			model.evtin=clkinput,..
// 			model.evtout=clkoutput,..
// 			model.firing=firing,..
// 			model.state=x,..
// 			model.dstate=Z,..
// 			model.odstate=odstate,..
// 			model.rpar=rpar,..
// 			model.ipar=ipar,..
// 			model.opar=opar,..
// 			model.blocktype='c',..
// 			model.dep_ut=dep_ut,..
// 			model.nzcross=nzcross,..
// 			model.nmode=nmode           
// 			
//   endfunction
// 
//   function x=standard_define( a, b, c, d )
//     x.a =  a;
//     x.b = b;
//     x.c = c;
//     x.d = d;
//   endfunction
// 
//   definecommand = "" + blockname + "(job=''define'',arg1=0,arg2=0);";
//   X = eval(definecommand);
//   cosblk = X.b;
//   cosblk.timestamp = getdate();
//   cosblk.blockname = blockname;
//   clear X;
// 
// endfunction
// 
// model=ortd_getcosblk(blockname="SuperBlock");
// save(blockname+"_scicosblockcfg.dat", model);


