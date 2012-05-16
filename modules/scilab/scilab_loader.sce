

// Interfacing functions are placed into this place



function [sim, out] = ld_scilab(sim, events, in, invecsize, outvecsize,  ...
    init_cmd, calc_cmd, destruct_cmd, scilab_path) // PARSEDOCU_BLOCK
// scilab - block
//
// in *+(invecsize) - input        scilab_interf.invec%d = [ .... ];
// out *+(outvecsize) - output        scilab_interf.outvec%d = [ .... ];
// scilab_path - Path to scilab5 executable
// 
// out = calc_cmd        scilab_interf.outvec%d = calc_cmd(scilab_interf.invec%d);
// 


  invecno=1; outvecno=1;

  btype = 22000;
  ipar = [invecsize; outvecsize; invecno; outvecno; 0; 0; 0; 0; 0; 0; 0; length(init_cmd); length(calc_cmd); ...
                   length(destruct_cmd); length(scilab_path); ascii(init_cmd)';...
                   ascii(calc_cmd)'; ascii(destruct_cmd)'; ascii(scilab_path)'; 0; ]; rpar = [];
  
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                   insizes=[invecsize], outsizes=[outvecsize], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction




function [sim, out] = ld_scilab2(sim, events, in, comp_fn, include_scilab_fns, scilab_path) // PARSEDOCU_BLOCK
// scilab2 - block
//
// A nicer interface to scilab. (See modules/scilab/demo/scilab_demo.sce for an example)
//
// in *+(invecsize) - input        scilab_interf.invec%d = [ .... ];
// out *+(outvecsize) - output        scilab_interf.outvec%d = [ .... ];
// 
// out = calc_cmd        scilab_interf.outvec%d = calc_cmd(scilab_interf.invec%d);
//
//
// comp_fn - scilab function that is executed online. Prototype:
//           function [block]=sample_comp_fn( block, flag )
//
//           flags are: 1 calc outputs, 4 initialise, 5 terminate, 10 configure I/O
//
// include_scilab_fns - unused for now. Set to list()
// scilab_path - Path to scilab5 executable
//
//
// 

  block.dummy = 0;

  // Get the number of I/O
  block = comp_fn(block, 10);

  if check_structure("block.invecsize") == %F then
    error("comp_fn did not return block.invecsize\n");
  end

  if check_structure("block.outvecsize") == %F then
    error("comp_fn did not return block.outvecsize\n");
  end

  invecsize = block.invecsize;
  outvecsize = block.outvecsize;

  // make source code from the computational function
  function_string = fun2string(comp_fn);
  function_string(1) = strsubst(function_string(1), "ans", "comp_fn"); // rename the function

  init_cmd="";

  LF = char(10);

  for i=1:length(length(function_string))
    init_cmd = init_cmd + function_string(i) + LF; // concate lines and add a line feed character
  end
//  init_cmd = init_cmd + "comp_fn = ans;" + LF;

  // Prepare initialisation string, which contains the comp_fn as source
  init_str = "" + LF + ...
             "block.invecsize=" + string(invecsize) + ";" + LF + ...
             "block.outvecsize=" + string(outvecsize) + ";" + LF + ...
             "block=comp_fn(block, 4);" + LF + ...
             "printf(''scilab computational function initialised\n''); " + LF + ...
             "block.inptr = list(1);" + LF + ...
             "block.outptr = list(1);" + LF;

  init_cmd = init_cmd + init_str;

  // Calc command
  calc_cmd = "block.inptr(1) = scilab_interf.invec1;" + LF + ...
             "block=comp_fn(block, 1); " + LF + ...
             "scilab_interf.outvec1 = block.outptr(1) " + LF + ... // without dot comma
             "" + LF + ...
             "" + LF;

  // command for destruction
  destruct_cmd = "" + LF + ...
             "block=comp_fn(block, 5);" + LF + ...
             "printf(''scilab computational function destructed\n''); " + LF + ...
             "clear block;" + LF;

//  fd = mopen('init_cmd.txt','wt');  mputl(init_cmd,fd);  mclose(fd);
//  fd = mopen('calc_cmd.txt','wt');  mputl(calc_cmd,fd);  mclose(fd);
//  fd = mopen('destruct_cmd.txt','wt');  mputl(destruct_cmd,fd);  mclose(fd);

//pause;

  // auf bereits bekannte funktion zurückführen
  [sim, out] = ld_scilab(sim, events, in, invecsize, outvecsize,  ...
    init_cmd, calc_cmd, destruct_cmd, scilab_path);

endfunction




