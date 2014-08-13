

// Interfacing functions are placed into this place

// printf("Hey, this is the ORTD-Toolbox\n");

function [sim, out] = ld_scilab(sim, events, in, invecsize, outvecsize, init_cmd, calc_cmd, destruct_cmd, scilab_path) // PARSEDOCU_BLOCK
// %PURPOSE: Block for interfacing scilab
//
// in *+(invecsize) - input        scilab_interf.invec%d = [ .... ];
// out *+(outvecsize) - output        scilab_interf.outvec%d = [ .... ];
// scilab_path - Path to scilab5 executable
// 
// out = calc_cmd        scilab_interf.outvec%d = calc_cmd(scilab_interf.invec%d);
//
// 


  invecno=1; outvecno=1;

  init_cmd = ascii(init_cmd);
  calc_cmd = ascii(calc_cmd);
  destruct_cmd = ascii(destruct_cmd);
  scilab_path = ascii(scilab_path);
  
  btype = 22000;
  ipar = [invecsize; outvecsize; invecno; outvecno; 0; 0; 0; 0; 0; 0; 0; length(init_cmd); length(calc_cmd); ...
                   length(destruct_cmd); length(scilab_path); (init_cmd)';...
                   (calc_cmd)'; (destruct_cmd)'; (scilab_path)'; 0; ]; rpar = [];
  
//   pause;
  
  [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, ...
                   insizes=[invecsize], outsizes=[outvecsize], ...
                   intypes=[ORTD.DATATYPE_FLOAT], outtypes=[ORTD.DATATYPE_FLOAT]  );

  // libdyn_conn_equation connects multiple input signals to blocks
  [sim,blk] = libdyn_conn_equation(sim, blk, list(in) );
  [sim,out] = libdyn_new_oport_hint(sim, blk, 0);   // 0th port
endfunction


// Computaional function example:
// 
//function [block]=sample_comp_fn( block, flag )
//  select flag
//    case 0 // update
//      printf("update states\n");
//
//    case 1 // output
//      printf("update outputs\n");
//      outvec = [1:6]';
//
//      block.outptr(1) = outvec;
//
//    case 4 // init
//      printf("init\n");
//
//    case 5 // terminate
//      printf("terminate\n");
//
//    case 10 // configure
//      printf("configure\n");
//      block.invecsize = 5;
//      block.outvecsize = 6;
//
//  end
//endfunction



function [sim, out] = ld_scilab2(sim, events, in, comp_fn, include_scilab_fns, scilab_path) // PARSEDOCU_BLOCK
// %PURPOSE: Block for calling a computational function written in scilab
//
// A nicer interface to scilab. (See modules/scilab/demo/scilab_demo.sce for an example)
//
// in *+(invecsize) - input
// out *+(outvecsize) - output
// 
// out = calc_cmd
//
//
// comp_fn - scilab function that is executed online. Prototype:
//           function [block]=sample_comp_fn( block, flag )
//
//           flags are: 1 calc outputs, 4 initialise, 5 terminate, 10 configure I/O
//
//           For further details have a look at the example.
//
// include_scilab_fns - unused for now. Set to list()
// scilab_path - Path to scilab5 executable. Use "BUILDIN_PATH" if you do not have special needs.
//
// NOTE: For now the flag "update states" is not called; Also a "reset states" flag is required.
//
//

  function [result] = check_structure( str )
    // give something like str="a.b" to see wheter this would lead to an error

    result = %T;
    err = execstr(str, 'errcatch');
    if (err ~= 0) then
      result = %F;
    end
  endfunction

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


  init_cmd="";
  LF = char(10);

  // look for any global variables; Could be used to build a set of variables that are then transfered
  // to the on-line scilab instance.
//   vars = macrovar(comp_fn); vars(3);  // however it also shows the nested functions

  // make source code from the computational function
  function_string = fun2string(comp_fn);
  function_string(1) = strsubst(function_string(1), "ans", "comp_fn"); // rename the function

  for i=1:length(length(function_string))
    init_cmd = init_cmd + function_string(i) + LF; // concate lines and add a line feed character
  end

// pause;

  // add all functions from include_scilab_fns
  for j=1:(length(include_scilab_fns)/2)


    fn = include_scilab_fns(  (j-1)*2 + 1  );
    tmp = fun2string(fn);

    fnname = include_scilab_fns( (j-1)*2 + 2 );
    tmp(1) = strsubst(tmp(1), "ans", fnname); // rename the function 

    for i=1:length(length(tmp))
      init_cmd = init_cmd + tmp(i) + LF; // concate lines and add a line feed character
    end
  end

// disp(init_cmd);

//   pause;


  // Prepare initialisation string, which contains the comp_fn in form of its source code
  init_str = "" + LF + ...
             "block.invecsize=" + string(invecsize) + ";" + LF + ...
             "block.outvecsize=" + string(outvecsize) + ";" + LF + ...
             "block=comp_fn(block, 4);" + LF + ...
             "printf(''scilab computational function initialised\n''); " + LF + ...
             "block.inptr = list(1);" + LF + ...
             "block.outptr = list(1);" + LF;

  init_cmd = init_cmd + init_str;

  // Calc command
  calc_cmd = "block.inptr(1) = scilab_interf.invec1;" + ...
             "block=comp_fn(block, 1); " + ...
             "scilab_interf.outvec1 = block.outptr(1); " + ... // all in one line
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

function [sim, out] = ld_scilab3(sim, events, in, comp_fn, include_scilab_fns, InitStr, scilab_path) // PARSEDOCU_BLOCK
// %PURPOSE: Block for calling a computational function written in scilab
//
// A nicer interface to scilab. (See modules/scilab/demo/scilab_demo.sce for an example)
//
// in *+(invecsize) - input
// out *+(outvecsize) - output
// InitStr - execute str in the begining. e.g. to define global variables
// 
// out = calc_cmd
//
//
// comp_fn - scilab function that is executed online. Prototype:
//           function [block]=sample_comp_fn( block, flag )
//
//           flags are: 1 calc outputs, 4 initialise, 5 terminate, 10 configure I/O
//
//           For further details have a look at the example.
//
// include_scilab_fns - unused for now. Set to list()
// scilab_path - Path to scilab5 executable. Use "BUILDIN_PATH" if you do not have special needs.
// 
// Nice feature: ld_scilab3 will look for variables that are not defined within comp_fn
//               and will try to transfer these variables from the Scilab instance running at
//               the development host to the on-line running Scilab instance.
//               This works only for single value variables only, though.
//
// NOTE: For now the flag "update states" is not called; Also a "reset states" flag is required.
//
//


   // check the input parameters
   ortd_checkpar(sim, list('String', 'InitStr', InitStr) );
   ortd_checkpar(sim, list('String', 'scilab_path', scilab_path) );
   // FIXME: add missing checks
   ortd_checkpar(sim, list('Signal', 'in', in) );

  function [result] = check_structure( str )
    // give something like str="a.b" to see wheter this would lead to an error

    result = %T;
    err = execstr(str, 'errcatch');
    if (err ~= 0) then
      result = %F;
    end
  endfunction

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

  // look for any global variables; Could be used to build a set of variables that are then transfered
  // to the on-line scilab instance.
  vars = macrovar(comp_fn);   V=vars(3); // however it also shows the nested functions
  printf("ld_scilab3: The following variables will be forwarded to the on-line Scilab instance:\n");

  ParamDefineStr = ""; // Code for the on-line Scilab to define the variables

  // go through all variables and build a list of all single value variables
  for i=1:size(V,1)
    
    tmp=0; 
     try
       estr = "ortd_checkpar(sim, list(''SingleValue'', '' '', " + V(i)  + " ) ); "
// printf("exec : %s\n", estr);
       execstr(estr); // the aborts the execution between try and catch, if V(i) is not a single value

       // get the value of the variable named V(i)
       estr = "tmp=" + V(i) + ";";
       // printf("exec : %s\n", estr);
       execstr(estr); // tmp=<V(i)>

       // printf("exec finished\n");
       printf(string(i) + ") " + V(i) + " = " + sprintf("%20.30f",tmp) + "\n");  // print an element of the list
       ParamDefineStr = ParamDefineStr + sprintf( V(i) + "=" + sprintf("%20.30f",tmp) + ";\n"); // build a list of assignment commands
     catch
       null;
       printf("NOTE: The variable named %s cannot be forwarded by now!\n", V(i));
     end
  end

//    printf("The command that will be executed on-line is:\n"); disp(ParamDefineStr);
  InitStr = ParamDefineStr + ";\n" + InitStr;

  init_cmd="";
  LF = char(10);

  // make source code from the computational function
  function_string = fun2string(comp_fn);
  function_string(1) = strsubst(function_string(1), "ans", "comp_fn"); // rename the function

  for i=1:length(length(function_string))
    init_cmd = init_cmd + function_string(i) + LF; // concate lines and add a line feed character
  end

  // add all functions from include_scilab_fns
  for j=1:(length(include_scilab_fns)/2)
    fn = include_scilab_fns(  (j-1)*2 + 1  );
    tmp = fun2string(fn);

    fnname = include_scilab_fns( (j-1)*2 + 2 );
    tmp(1) = strsubst(tmp(1), "ans", fnname); // rename the function 

    for i=1:length(length(tmp))
      init_cmd = init_cmd + tmp(i) + LF; // concate lines and add a line feed character
    end
  end



  // Prepare initialisation string, which contains the comp_fn in form of its source code
  init_str = "" + LF + ...
             "block.invecsize=" + string(invecsize) + ";" + LF + ...
             "block.outvecsize=" + string(outvecsize) + ";" + LF + ...
	      InitStr + ";" + LF + ...
             "block=comp_fn(block, 4);" + LF + ...
             "printf(''scilab computational function initialised\n''); " + LF + ...
             "block.inptr = list(1);" + LF + ...
             "block.outptr = list(1);" + LF;

  init_cmd = init_cmd + init_str;

  // Calc command
  calc_cmd = "block.inptr(1) = scilab_interf.invec1;" + ...
             "block=comp_fn(block, 1); " + ...
             "scilab_interf.outvec1 = block.outptr(1); " + ... // all in one line
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



function [sim, out] = ld_scilab4(sim, events, in, invecsize, outvecsize, comp_fn, ForwardVars, par) // PARSEDOCU_BLOCK
// %PURPOSE: Block for calling a computational function written in scilab
//
// A nicer interface to scilab. (See modules/scilab/demo/scilab_demo.sce for an example)
//
// in *+(invecsize) - input
// out *+(outvecsize) - output
// 
// out = calc_cmd
//
//
// comp_fn - scilab function that is executed online. Prototype:
//           function [block]=sample_comp_fn( block, flag )
//
//           flags are: 1 calc outputs, 4 initialise, 5 terminate
//
//           For further details have a look at the example.
//
// ForwardVars - %t or %f Forward variables to the online Scilab instance
// 
// The structure par must contain at least the following elements:
// 
// include_scilab_fns - list( fn1, "fn1", fn2, "fn2" ) , whereby fn1 and fn2 stand for Scilab functions
// scilab_path - Path to scilab5 executable. Use "BUILDIN_PATH" if you do not have special needs.
// InitStr - execute str in the begining. e.g. to define global variables
// 
// Nice feature: ld_scilab4 will look for variables that are not defined within comp_fn, if ForwardVars = %t
//               and will try to transfer these variables from the Scilab instance running at
//               the development host to the on-line running Scilab instance.
//               This works only for single value variables only, though.
//
// NOTE: For now the flag "update states" is not called; Also a "reset states" flag is required.
//
//

  // additional parameters
  include_scilab_fns = par.include_scilab_fns;
  InitStr = par.InitStr;
  scilab_path = par.scilab_path;


   // check the input parameters
   ortd_checkpar(sim, list('String', 'InitStr', InitStr) );
   ortd_checkpar(sim, list('String', 'scilab_path', scilab_path) );
   // FIXME: add missing checks
   ortd_checkpar(sim, list('Signal', 'in', in) );

  function [result] = check_structure( str )
    // give something like str="a.b" to see wheter this would lead to an error

    result = %T;
    err = execstr(str, 'errcatch');
    if (err ~= 0) then
      result = %F;
    end
  endfunction


// 
// Rev ld_scilab4: removed I/O configuration by the scilab comp_fn
// 

//   block.dummy = 0;
// 
//   // Get the number of I/O
//   block = comp_fn(block, 10);
// 
//   if check_structure("block.invecsize") == %F then
//     error("comp_fn did not return block.invecsize\n");
//   end
// 
//   if check_structure("block.outvecsize") == %F then
//     error("comp_fn did not return block.outvecsize\n");
//   end
// 
//   invecsize = block.invecsize;
//   outvecsize = block.outvecsize;


  if ForwardVars == %t then
    // look for any global variables; Could be used to build a set of variables that are then transfered
    // to the on-line scilab instance.
    vars = macrovar(comp_fn);   V=vars(3); // however it also shows the nested functions
    printf("ld_scilab3: The following variables will be forwarded to the on-line Scilab instance:\n");

    ParamDefineStr = ""; // Code for the on-line Scilab to define the variables

    // go through all variables and build a list of all single value variables
    for i=1:size(V,1)
      
      tmp=0; 
      try
	estr = "ortd_checkpar(sim, list(''SingleValue'', '' '', " + V(i)  + " ) ); "
  // printf("exec : %s\n", estr);
	execstr(estr); // the aborts the execution between try and catch, if V(i) is not a single value

	// get the value of the variable named V(i)
	estr = "tmp=" + V(i) + ";";
	// printf("exec : %s\n", estr);
	execstr(estr); // tmp=<V(i)>

	// printf("exec finished\n");
	printf(string(i) + ") " + V(i) + " = " + sprintf("%20.30f",tmp) + "\n");  // print an element of the list
	ParamDefineStr = ParamDefineStr + sprintf( V(i) + "=" + sprintf("%20.30f",tmp) + ";\n"); // build a list of assignment commands
      catch
	null;
	printf("NOTE: The variable named %s cannot be forwarded by now!\n", V(i));
      end
    end

  else
    ParamDefineStr = "";
  end


    LF = char(10);
  //    printf("The command that will be executed on-line is:\n"); disp(ParamDefineStr);
    InitStr = InitStr + ParamDefineStr + ";" + LF;

    CompFnStr="";

    // make source code from the computational function
    function_string = fun2string(comp_fn);
    function_string(1) = strsubst(function_string(1), "ans", "comp_fn"); // rename the function

    for i=1:length(length(function_string))
      CompFnStr = CompFnStr + function_string(i) + LF; // concate lines and add a line feed character
    end

    // add all functions from include_scilab_fns
    for j=1:(length(include_scilab_fns)/2)
      fn = include_scilab_fns(  (j-1)*2 + 1  );
      tmp = fun2string(fn);

      fnname = include_scilab_fns( (j-1)*2 + 2 );
      tmp(1) = strsubst(tmp(1), "ans", fnname); // rename the function 

      for i=1:length(length(tmp))
	CompFnStr = CompFnStr + tmp(i) + LF; // concate lines and add a line feed character
      end
    end



  // Prepare initialisation string, which contains the comp_fn in form of its source code
  init_str = "" + LF + ...
             "block.invecsize=" + string(invecsize) + ";" + LF + ...
             "block.outvecsize=" + string(outvecsize) + ";" + LF + ...
	      InitStr + ";" + LF + ...
             "block=comp_fn(block, 4);" + LF + ...
             "printf(''scilab computational function initialised\n''); " + LF + ...
             "block.inptr = list(1);" + LF + ...
             "block.outptr = list(1);" + LF;

  init_cmd = CompFnStr + init_str;



//   disp(init_cmd);

  // Calc command
  calc_cmd = "block.inptr(1) = scilab_interf.invec1;" + ...
             "block=comp_fn(block, 1); " + ...
             "scilab_interf.outvec1 = block.outptr(1); " + ... // all in one line
             "" + LF;

  // command for destruction
  destruct_cmd = "" + LF + ...
             "block=comp_fn(block, 5);" + LF + ...
             "printf(''scilab computational function destructed\n''); " + LF + ...
             "clear block;" + LF;

//  fd = mopen('init_cmd.txt','wt');  mputl(init_cmd,fd);  mclose(fd);
//  fd = mopen('calc_cmd.txt','wt');  mputl(calc_cmd,fd);  mclose(fd);
//  fd = mopen('destruct_cmd.txt','wt');  mputl(destruct_cmd,fd);  mclose(fd);


  // auf bereits bekannte funktion zurückführen
  [sim, out] = ld_scilab(sim, events, in, invecsize, outvecsize,  ...
    init_cmd, calc_cmd, destruct_cmd, scilab_path);

endfunction
