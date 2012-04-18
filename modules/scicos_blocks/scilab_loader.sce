

// Interfacing functions are placed in this place





// An example could be the following scilab function. PARSEDOCU_BLOCK is a hint for the documentation generator to generate documentation for this block
function [sim, out] = ld_scicosblock(sim, events, in, cosblk) // PARSEDOCU_BLOCK
// 
// ld_scicosblock - include a scicos block / schematic (EXPERIMENTAL)
//
// in * - input
// out * - output
// 
// cosblk - structure as loaded by ortd_getcosblk 
//

  

  model = cosblk;

  identstr = model.blockname;

  printf("Including Scicos Block %s\n", identstr);
  printf("Make sure that the C function %s() is included in the binary used for execution\n", identstr);

  
  if ( length(cosblk.in) ~= 1 ) then
    error("Only one input port to the Scicos-Block allowed at the moment");
  end

  if ( length(cosblk.out) ~= 1 ) then
    error("Only one output port to the Scicos-Block allowed at the moment");
  end


  Nin = cosblk.in(1);
  Nout = cosblk.out(1);


  parlist = new_irparam_set();

  parlist = new_irparam_elemet_ivec(parlist, ascii(identstr), 10);
  parlist = new_irparam_elemet_ivec(parlist, model.ipar, 11);
  parlist = new_irparam_elemet_rvec(parlist, model.rpar, 12);
  parlist = new_irparam_elemet_rvec(parlist, model.dstate, 13);

  blockparam = combine_irparam(parlist);



 btype = 15200 + 0;; // the same id you are giving via the "libdyn_compfnlist_add" C-function
//  [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ 0, Nin, Nout, length(identstr), ascii(identstr)  ], rpar=[  ], ...
//                   insizes=[Nin], outsizes=[Nout], ...
//                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

 [sim,blk] = libdyn_new_block(sim, events, btype, ipar=[ 0, Nin, Nout, blockparam.ipar(:)'  ], rpar=[ blockparam.rpar ], ...
                  insizes=[Nin], outsizes=[Nout], ...
                  intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

 [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
 [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction





function cosblk=ortd_getcosblk(blockname, pathtoscifile)
  //
  // Extract information from Scicos block interfacing function macros (*.sci) files
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



// model=ortd_getcosblk(blockname="SuperBlock");
// save(blockname+"_scicosblockcfg.dat", model);


