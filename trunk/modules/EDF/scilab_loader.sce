
function [sim] = ld_EDFWrite(sim, events, fname, smp_freq, inlist, innames, PhyMax, PhyMin, DigMin, DigMax) // PARSEDOCU_BLOCK
// Write data to EDF+ - Files
//
// inlist - list() of signals of size 1
// 
// 


  Nin = length(inlist);

  // Nr of channels
  chns = Nin;

  if chns ~= length(innames) then
    error("chns ~= length(innames)");
  end
  if chns ~= length(PhyMax) then
    error("chns ~= length(PhyMax)");
  end
  if chns ~= length(PhyMin) then
    error("chns ~= length(PhyMin)");
  end
  if chns ~= length(DigMax) then
    error("chns ~= length(DigMax)");
  end
  if chns ~= length(DigMin) then
    error("chns ~= length(DigMin)");
  end


// introduce some parameters that are refered to by id's
parameter1 = 12345;
vec = [ chns, smp_freq ];

   parlist = new_irparam_set();

   parlist = new_irparam_elemet_ivec(parlist, parameter1, 10); 
   parlist = new_irparam_elemet_ivec(parlist, vec, 11); // vector of integers (double vectors are similar, replace ivec with rvec)
   parlist = new_irparam_elemet_ivec(parlist, ascii(fname), 12);

   parlist = new_irparam_elemet_ivec(parlist, DigMax, 13); 
   parlist = new_irparam_elemet_ivec(parlist, DigMin, 14); 
   parlist = new_irparam_elemet_rvec(parlist, PhyMax, 15); 
   parlist = new_irparam_elemet_rvec(parlist, PhyMin, 16); 

   for i=1:Nin
     parlist = new_irparam_elemet_ivec(parlist, ascii( innames(i) ), 100+i-1);
   end

   p = combine_irparam(parlist);

// Set-up the block parameters and I/O ports

  Uipar = [ p.ipar ];
  Urpar = [ p.rpar ];
  btype = 15800 + 0; // Reference to the block's type (computational function). Use the same id you are giving via the "libdyn_compfnlist_add" C-function

  insizes=[ones( Nin,1 ) ];
  outsizes=[]; 
  dfeed=[];  // for each output 0 (no df) or 1 (a direct feedthrough to one of the inputs)
  intypes=  [ ones( Nin,1 )*ORTD.DATATYPE_FLOAT ];
  outtypes=[];

  blocktype = 1; // 1-BLOCKTYPE_DYNAMIC, 2-BLOCKTYPE_STATIC

  ///////////////
  [sim, blk] = libdyn_CreateBlockAutoConfig(sim, events, btype, blocktype, Uipar, Urpar, insizes, outsizes, intypes, outtypes, dfeed)
  /////////////
  
 [sim,blk] = libdyn_conn_equation(sim, blk, inlist );
//  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction