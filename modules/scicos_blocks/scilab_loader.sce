

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





function cosblk=ortd_getcosblk(blockname, pathtoscifile)  // PARSEDOCU_BLOCK
  //
  // %PURPOSE: Extract information from Scicos block interfacing function macros (*.sci) files
  //
  // pathtoscifile - The interfacing function macro (a *.sci file)
  //


  //exec(blockname + '_c.sci');
  exec(pathtoscifile);

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

function cosblk=ortd_getcosblk2(blockname, pathtoscifile)  // PARSEDOCU_BLOCK
  //
  // %PURPOSE: Extract information from Scicos block interfacing function macros (*.sci) files
  //
  // pathtoscifile - The interfacing function macro (a *.sci file)
  //


  //exec(blockname + '_c.sci');
  exec(pathtoscifile);

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

  definecommand = "" + blockname + "(job=''define'',arg1=0,arg2=0);";
  X = eval(definecommand);
  cosblk = X.b;
  cosblk.timestamp = getdate();
  cosblk.blockname = blockname;
  clear X;

endfunction

// model=ortd_getcosblk(blockname="SuperBlock");
// save(blockname+"_scicosblockcfg.dat", model);


