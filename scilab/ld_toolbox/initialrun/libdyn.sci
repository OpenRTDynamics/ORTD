//
//    Copyright (C) 2010, 2011, 2012, 2013, 2013, 2014, 2015, 2016  Christian Klauer
//
//    This file is part of OpenRTDynamics, the Real-Time Dynamics Framework
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





//
// in function libdyn_setup_sch2: add datatype checking
//
//
// TODO: nur noch die Objectliste in sim structur über vom beutzer übergebene oid verwenden
// 2) Alle Objectkopien, die an den Benutzer zurückgegeben werden mit verknüpfung zu sim struktur versehen
//    und später überürüfen, ob die übergebenen objekte zur übergebenen simulation passen
// 3) Fehler melden, wenn Blöcke schon verbunden sind; dies in libdyn_connect_block realisieren
//
// 4) Port Hint Objects können nicht mit den Simulationsausgängen verbunden werden! DONE
// 5) libdyn_check_object könnte objecte die mit list(obj) eingepackt sind auspacken, wenn es sich um einer Liste mit einem Object handelt. (Kompatiblität)
// 6) list(obj, 0...) aus shortcuts rausnehmen um warnungen zu verhindern
// 7) libdyn_close_loop sollte prüfen, ob auch tatsächlich eine Feedback variable übergeben wird, die von libdyn_new_feedback erzeugt wurde


// 
// Datatypes
// 

ORTD.DATATYPE_UNCONFIGURED = 0;
ORTD.DATATYPE_FLOAT = (1 + (8 * 2^5));
ORTD.DATATYPE_SHORTFLOAT = 4;
ORTD.DATATYPE_INT32 = 2;
ORTD.DATATYPE_BOOLEAN = 3;
ORTD.DATATYPE_EVENT = 5;
ORTD.DATATYPE_BINARY = 6;

// #define DATATYPE_UNCONFIGURED 0
// #define DATATYPE_FLOAT (1 | (sizeof(double) << 5))
// #define DATATYPE_SHORTFLOAT 4
// #define DATATYPE_INT32 2
// #define DATATYPE_BOOLEAN 3
// #define DATATYPE_EVENT 5
// #define DATATYPE_BINARY 6
// #define DATATYPE_UINT32 7
// #define DATATYPE_INT16 8
// #define DATATYPE_UINT16 9
// #define DATATYPE_INT8 10
// #define DATATYPE_UINT8 11


//
// Some terminal color codes for usage with ld_printf
//

ORTD.termcode.clearscreen = ascii(27) + '[2J';

ORTD.termcode.black = ascii(27) + '[30m';
ORTD.termcode.red = ascii(27) + '[31m';
ORTD.termcode.green = ascii(27) + '[32m';
ORTD.termcode.yellow = ascii(27) + '[33m';
ORTD.termcode.blue = ascii(27) + '[34m';
ORTD.termcode.magenta = ascii(27) + '[35m';
ORTD.termcode.cyan = ascii(27) + '[36m';
ORTD.termcode.white = ascii(27) + '[37m';

ORTD.termcode.redbold = ascii(27) + '[31;1m';
ORTD.termcode.greenbold = ascii(27) + '[32;1m';
ORTD.termcode.yellowbold = ascii(27) + '[33;1m';
ORTD.termcode.bluebold = ascii(27) + '[34;1m';

ORTD.termcode.reset = ascii(27) + '[0m';




// Constants for describing thread types
ORTD.ORTD_RT_REALTIMETASK = 1;
ORTD.ORTD_RT_NORMALTASK = 2;

ORTD.FASTCOMPILE = %t; // if %t then some optional checks are skipped





// Globale variable which gives each simulation a unique and random id
// this id is stored in every object passed to the user, so further calls
// to this obj can check, wheter it belongs to the simulation passed
global libdyn_simu_id_counter;
libdyn_simu_id_counter = 1000 + ceil(rand()*10000);






function ortd_checkpar(sim, l)
// 
// Check for several datatypes used for parameters of the blocks
// 

//       printf("Checking signal %s\n", l(2) );

  select l(1)
    case 'Signal' then
      try
        libdyn_check_object(sim,l(3));
      catch
        printf("Unexpected type of the parameter ''%s''. Expected was a signal.\n", l(2)); error('.');
      end

    case 'SignalList' then
      try
        if (type(l(3)) == 15) then // is this a list?
          List = l(3);
          for i=1:length(List) 
            libdyn_check_object(sim, List(i) ); // Are all entries signals?
          end
        else
          error(".");
        end
      catch
        printf("Unexpected type of the parameter ''%s''. Expected was a list() of signals.\n", l(2)); error('.');
      end

    case 'SingleValue' then
      if (type(l(3)) ~= 1) | (length(l(3)) ~= 1) then
        printf("Unexpected type of the parameter ''%s''. Expected was a single value.\n", l(2)); error('.');
      end
      
    case 'Vector' then
      if (type(l(3)) ~= 1)  then
        printf("Unexpected type of the parameter ''%s''. Expected was a vector.\n", l(2)); error('.');
      end

    case 'String' then
      if (type(l(3)) ~= 10) then
        printf("Unexpected type of the parameter ''%s''. Expected was a string.\n", l(2)); error('.');
      end

    else
//       error("Unknown type to check");
      null;
  end
endfunction


// 
// get datatype size
// 
// counterpart to the C-function int libdyn_config_get_datatype_len(int datatype)
// in libdyn.c
// 
function NBytes=libdyn_datatype_len(datatype)
  NBytes = 0;

  select datatype
    case ORTD.DATATYPE_FLOAT
      NBytes = 8;

    case ORTD.DATATYPE_SHORTFLOAT
      NBytes = 4;

    case ORTD.DATATYPE_INT32
      NBytes = 4;

    case ORTD.DATATYPE_BINARY
      NBytes = 1;
  end

  if NBytes == 0 then
    error("Unknown datatype");
  end

endfunction


// new schematic
function sim = libdyn_new_simulation(insizes, outsizes)
  sim_struct.insizes = insizes; // Vektor of insizes
  sim_struct.outsizes = outsizes; // Vektor of outsizes
  sim_struct.parlist = new_irparam_set(); // irparam container
  
  // used by libdyn_new_blockid() to create new block ids
  sim_struct.objectidcounter = 200; // also used for irpar ids, never start at 0 because its is reserverd for external in/out
  
  sim_struct.cllist = list(); // Connection list is stored here
//  sim_struct.clist_count = 0;

  sim_struct.objectlist = list(); // Contains all block etc...

  global libdyn_simu_id_counter;
  sim_struct.simid = libdyn_simu_id_counter; // a unique identifier for each simulation struct
  libdyn_simu_id_counter = libdyn_simu_id_counter + 1;

  sim = sim_struct;
endfunction

// New version forked form above as of 16.2.14 to add in and outtypes
function sim = libdyn_new_simulation2(insizes, outsizes, intypes, outtypes)
  global libdyn_simu_id_counter;

//  sim_struct.insizes = insizes; // Vektor of insizes
//  sim_struct.outsizes = outsizes; // Vektor of outsizes
//  sim_struct.intypes = intypes;
//  sim_struct.outtypes = outtypes;
//
//  sim_struct.parlist = new_irparam_set(); // irparam container
//  
//  // used by libdyn_new_blockid() to create new block ids
//  sim_struct.objectidcounter = 200; // also used for irpar ids, never start at 0 because its is reserverd for external in/out
//  
//  sim_struct.cllist = list(); // Connection list is stored here
////  sim_struct.clist_count = 0;
//
//  sim_struct.objectlist = list(); // Contains all block etc...
//  
//  sim_struct.simid = libdyn_simu_id_counter; // a unique identifier for each simulation struct
//  
  
  sim_struct = struct( 'insizes', insizes, 'outsizes', outsizes, 'intypes', intypes, 'outtypes', outtypes, 'parlist', new_irparam_set(), 'objectidcounter', 200, 'cllist', list(), 'objectlist', list(), 'simid', libdyn_simu_id_counter );
  
  
  libdyn_simu_id_counter = libdyn_simu_id_counter + 1;

  sim = sim_struct;
endfunction



// creates a new unique block identifier, which is also used as id for saving irpar parameters
function [sim,oid] = libdyn_new_objectid(sim); 
  oid = sim.objectidcounter;
  sim.objectidcounter = sim.objectidcounter + 1;
endfunction


// blocks that may be added by the user
// btype is the kind of block - the comp fn is determined based on this integer
// FIXME: This function is obsolete by now and should be replaced by libdyn_new_block
function [sim,blk] = libdyn_new_blk_generic(sim, events, btype, ipar, rpar, insizes, outsizes, intypes, outtypes)
  [sim,oid] = libdyn_new_objectid(sim); // get new object id
  
//  blk.oid = oid;
//  blk.simid = sim.simid;
//  blk.objecttype = 0; // Which object type is this? 0 - Block
//  blk.input_block = 0;  // FIXME raus!
//  blk.magic = 678234;
//  blk.btype = btype;

//  printf("*****************\n");
    
  blk = struct( 'oid', oid, 'simid', sim.simid, 'objecttype', 0, 'input_block', 0, 'magic', 678234, 'btype', btype );

  id = oid; // id for this parameter set - take the object id in this case as it is unique
  Nbipar = length(ipar);
  Nbrpar = length(rpar);
  eventlist_len = length(events);
  
  header = [ btype; oid; Nbipar; Nbrpar; eventlist_len ]; // add header parameters
  header = [ header; events(:) ]; // Add eventlist

  if (exists('insizes') == %T) then
    printf("insizes exisi\n");
    blk.insizes = insizes;
    blk.outsizes = outsizes;
    blk.intypes = intypes;
    blk.outtypes = outtypes;
  else
//     printf("no insizes exisi\n"); // put out a warning later
  end
  
  // store parameters
  sim.parlist = new_irparam_elemet(sim.parlist, id, IRPAR_LIBDYN_BLOCK, [header; ipar(:)], [rpar(:)]);
  
  // stort block structure in objectlist
  sim.objectlist(oid) = blk;
endfunction

// blocks that may be added by the user
// btype is the kind of block - the comp fn is determined based on this integer
function [sim,blk] = libdyn_new_block(sim, events, btype, ipar, rpar, insizes, outsizes, intypes, outtypes)
  [sim,oid] = libdyn_new_objectid(sim); // get new object id
  
  
//  blk.oid = oid;
//  blk.simid = sim.simid;
//  blk.objecttype = 0; // Which object type is this? 0 - Block
//  blk.input_block = 0;  // FIXME raus!
//  blk.magic = 678234;
//  blk.btype = btype;
//
//  // set in and output sizes and types
//  blk.insizes = insizes;
//  blk.outsizes = outsizes;
//  blk.intypes = intypes;
//  blk.outtypes = outtypes;
  
  blk = struct( 'oid', oid, 'simid', sim.simid, 'objecttype', 0, 'input_block', 0, 'magic', 678234, 'btype', btype, 'insizes', insizes, 'outsizes', outsizes, 'intypes', intypes, 'outtypes', outtypes  );
  
  

  id = oid; // id for this parameter set - take the object id in this case as it is unique
  Nbipar = length(ipar);
  Nbrpar = length(rpar);
  eventlist_len = length(events);
  
  header = [ btype; oid; Nbipar; Nbrpar; eventlist_len ]; // add header parameters
  header = [ header; events(:) ]; // Add eventlist


  
  // store parameters
  sim.parlist = new_irparam_elemet(sim.parlist, id, IRPAR_LIBDYN_BLOCK, [header; ipar(:)], [rpar(:)]);
  
  // stort block structure in objectlist
  sim.objectlist(oid) = blk;
endfunction


// test wheter the given object is a libdyn object
function [ret] = libdyn_is_ldobject(obj) 
    
  ret = %F;
  if typeof(obj) == 'st' then
    if isfield(obj, 'magic') then
      if obj.magic == 678234 then
        ret = %T;
      end
    end
  end

endfunction

// Check wheter the object given by the user is part of the given simulation
function libdyn_check_object(sim,obj) 
  if libdyn_is_ldobject(obj) == %F then
    printf("The given variable is no libdyn object. Insted, the following data is contained in this variable:\n");
    disp(obj);
    error("");
  end

  if obj.simid ~= sim.simid then
    printf("Object does not belong to the simulation-context described by the <sim> structure\n");
    error("");
  end
endfunction


                // Check wheter the object given by the user is part of the given simulation
                function ret=libdyn_check_object2(sim,obj)
                    //
                    // Additional return value in comparision to libdyn_check_object
                    //
                    ret = 1; 
                    if libdyn_is_ldobject(obj) == %F then
                        printf("The given variable is no libdyn object. Instead, the following data is contained in this variable:\n");
                        disp(obj);
                        ret = -1;
                        return;
                    end

                    if obj.simid ~= sim.simid then
                        printf("Object does not belong to the simulation-context described by the <sim> structure\n");
                        ret = -1;
                        return;
                    end
                endfunction





// try to find a libdyn object within a list() 
// usage for compatibility issues
function [obj] = libdyn_extrakt_obj( whatever )
  obj = [];

  if libdyn_is_ldobject(whatever) then
    obj = whatever;
  else


    if typeof(whatever) == 'list' then
      if libdyn_is_ldobject( whatever(1) ) then
        obj = whatever(1);
      end
    end

  end
endfunction


// FIXME: Grosser Fehler! [sim,blk] muss es jetzt heissen
// allerdings müssen alle Schaltbilder angepasst werden
// Diese Funktion sollte nun obsolet sein
function [blk] = libdyn_get_input_signals(sim);
  [sim,oid] = libdyn_new_objectid(sim); // get new object id
  blk.oid = oid;
  blk.simid = sim.simid;

  blk.objecttype = 1; // Which object type is this? 1 - External simulation box inputs
  blk.outsizes = [];
  
  blk.input_block = 1; // Because the Simulations inputs are not a real block // FIXME: raus

  blk.magic = 678234;  

  // store block structure
  sim.objectlist(oid) = blk;
endfunction

// Deswegen hier ein neuer name
function [sim,blk] = libdyn_get_external_ins(sim);
  [sim,oid] = libdyn_new_objectid(sim); // get new object id
  blk.oid = oid;
  blk.simid = sim.simid;

  blk.objecttype = 1; // Which object type is this? 1 - External simulation box inputs
  blk.outsizes = [];
  
  blk.input_block = 1; // Because the Simulations inputs are not a real block // FIXME: raus
  
  blk.magic = 678234;
  // store block structure
  sim.objectlist(oid) = blk;
endfunction


// Create new object, which refers to a certain output port of a block or an external input
// Mainly this is a reference to an higherlevel object + a given port number
// the returned object may also named "signal"
function [sim,blk] = libdyn_new_oport_hint(sim, object, port);
  if object.objecttype ~= 0 & object.objecttype ~= 1 then 
    // The object is neither a block nor a simulation input
    error('The object is neither a block nor a simulation input');
  end
  
  
  [sim,oid] = libdyn_new_objectid(sim); // get new object id
  
//  blk.oid = oid;
//  blk.simid = sim.simid;
//
//  blk.objecttype = 4; // Which object type is this? 4 - Special output port hint object
//  blk.outsizes = [];
//  
//  // ---- special vars of this object class -----
//  blk.highleveloid = object.oid;
//  blk.highlevelotype = object.objecttype;
//  blk.outport = port;
//
//  blk.magic = 678234;



  blk = struct( 'oid', oid, 'simid', sim.simid, 'objecttype', 4, 'outsizes', [], 'highleveloid', object.oid, 'highlevelotype', object.objecttype, 'outport', port, 'magic', 678234 );
  
  // stort block structure
  sim.objectlist(oid) = blk;
endfunction


//
// Easy creation of feedback loops UNTESTED so far
//

// Get destination info of the feedback
function [dblk, dport] = libdyn_extrakt_fbdummy(sim, fbdummy)
  // object aus liste laden
  oid = fbdummy.oid;
  tmp = sim.objectlist(oid);
  
  // extract feedback destination
  dblk = tmp.dblk;
  dport = tmp.dport;

  if (dblk == -1) then
    printf("Error: You created created a feedback variable, but it is not connected to a anything.\n");
    error(".");
  end

  
//  printf("extrakt feedback dblk.oid=%d fbdummy.oid=%d\n", dblk.oid, oid);
endfunction

// Write information pn where to connect a feedback dummy reference. The information contains the block
// plus port number of the destination of the feedback loop
function [sim] = libdyn_fill_feedback(sim, fbdummy, dblk, dport)
  libdyn_check_object(sim,fbdummy);
  
  oid = fbdummy.oid;

  // get object data  
  tmp = sim.objectlist(oid);

  // modify data
  if (tmp.dblk == -1) then 
    tmp.dblk = dblk;
    tmp.dport = dport;
  else
    printf("This feeback variable is already used. (You can only use it once)\n");
    printf("This is actually a stupid a limitation that will be fixed in the\n");
    printf("furture. As a workaround, apply the feedback variable to a gain of one\n");
    printf("and use the output of the gain-block for multiple connections.\n");
    printf("Sorry for this ;-)\n");
    error(".");
  end

  // write back
  sim.objectlist(oid) = tmp;
endfunction


// Create new object, which refers to a certain output port of a block or an external input
// Mainly this is a reference to an higherlevel object + a given port number
// the returned object may also named "signal"
function [sim,fbdummy] = libdyn_new_feedback(sim);
  [sim,oid] = libdyn_new_objectid(sim); // get new object id
//  fbdummy.oid = oid;
//  fbdummy.simid = sim.simid;
//
//  fbdummy.objecttype = 8; // Which object type is this? 8 - Special feedback dummy referer
//  
//  fbdummy.magic = 678234;
//
//  // later: a list of connections to some blocks or whatever
//  fbdummy.dblk = -1; // -1 means uninitialised
//  fbdummy.dport = -1; // -1 means uninitialised
//  
  fbdummy = struct( 'oid', oid, 'simid', sim.simid, 'objecttype', 8, 'magic', 678234, 'dblk', -1, 'dport', -1  );
  

  // ---- special vars of this object class -----
  
  // stort block structure -- fbdummy is only a reference to that
  sim.objectlist(oid) = fbdummy;
  
//    printf("libdyn: New feedback dummy created oid=%d\n", oid);
endfunction

// connects src port hint obj to the block/port refered by dest_fb_dummy
function [sim] = libdyn_close_loop(sim, src, dest_fb_dummy)
// printf("libdyn closing loop\n");
  libdyn_check_object(sim,dest_fb_dummy);
  libdyn_check_object(sim,src);
 
  [sblk, sport] = libdyn_deref_porthint(sim, src);
  [dblk, dport] = libdyn_extrakt_fbdummy(sim, dest_fb_dummy);

  sim = libdyn_connect_block(sim, sblk, sport, dblk, dport); 
endfunction



// Get block and port information from port hint object
function [blk, port] = libdyn_deref_porthint(sim, obj)  
  // libdyn_check_object has to be run on obj
  
      highoid = obj.highleveloid; // Hole object id des Referenzierten objects
      port = obj.outport; // Hole die portnummer des Referenzierten objects
      
//      if (obj.simid ~= sim.simid) then // tested on upper level
//        error("Object does not belong to this simulation");
//      end

      // FIXME Hier kann getestet werden, ob der index existiert. Wenn nicht
      // ist es wahrscheinlich, dass [sim,obj] = ... nicht verwendet wurde sondern
      // sim weggelassen wurde
      try
        blk = sim.objectlist(highoid); // Ersetze das Quell-Object
        
      catch
        printf("The signal identifier highleveloid (Object ID of a block) = %d is not known to the internal list\n", highoid);
        printf("Propably you forgot the >sim< in [sim,obj] = ...\n");
        error("");
      end
endfunction



//
// Connecting functions
//

// Block to Block connection
//
// src_port and dst_port are integers used as index. counting starts from 0
function sim = libdyn_connect_block(sim, src, src_port, dst, dst_port)
  if src.magic ~= 678234 | dst.magic ~= 678234 then
    printf("bad block magic!");
    error("");
  end
  
  libdyn_check_object(sim,src);
  libdyn_check_object(sim,dst);
  
  
  //
  // test port sizes and types
  //
  if (isfield(src, "outsizes") == %T) & (isfield(dst, "insizes") == %T) then
      // test for port sizes is possible -- otherwise the interpreter lib has to check
      
//      printf("checking port sizes\n");
//      src.outsizes(src_port+1);
      
//      disp(dst.insizes);
//      disp(dst_port+1);
//      dst.insizes(dst_port+1);
    try
      
      if src.outsizes(src_port+1) ~= dst.insizes(dst_port+1) then
         printf("Error connecting port sizes %d --> %d\n", src.outsizes(src_port+1) , dst.insizes(dst_port+1) );
         printf("Incorrect port sizes");
         error("");
      end
    catch 
        printf("There was an error concerning the portsize checking. Check wheter you are connection to much input ports to a block. If not, also an interfacing function could have maldefined some input / output sizes\n");
        error("");
    end
    
  end    

  if (isfield(src, "outtypes") == %T) & (isfield(dst, "intypes") == %T) then
      // test for port sizes is possible -- otherwise the interpreter lib has to check
      
  //    printf("checking port types\n");
    try      
      if src.outsizes(src_port+1) ~= dst.insizes(dst_port+1) then
         printf("Error connecting port types %d --> %d\n", src.outtypes(src_port+1) , dst.intypes(dst_port+1) );
         printf("Incorrect port types");
         error("");
      end
    catch 
        printf("There was an error concerning the porttypes checking. Check wheter you are connection to much input ports to a block. If not, also an interfacing function could have maldefined some input / output sizes\n");
        error("");
        
    end
      
  end    

  
  // Add the new connection to the connectionlist
  src_id = src.oid;
  dst_id = dst.oid;
  
  sim.cllist($+1) = [0, src_id, src_id, src_port,   0, dst_id, dst_id, dst_port]; // add new line
endfunction

// Connection to external inputs
function sim = libdyn_connect_extern_in(sim, src_port, dst, dst_port)
  if dst.magic ~= 678234 then
    printf("bad block magic!");
    error("");
  end
  libdyn_check_object(sim,dst);


  // test port sizes and types
  if (isfield(dst, "insizes") == %T) then
      // test for port sizes is possible -- otherwise the interpreter lib has to check
      
//      printf("checking port sizes inputs of the simulation\n");
      
      if sim.insizes(src_port+1) ~= dst.insizes(dst_port+1) then
         printf("Error connecting port sizes %d --> %d\n", sim.insizes(src_port+1) , dst.insizes(dst_port+1) );
         printf("Incorrect port sizes for simulation inputs");
         error("");
      end
  end    


  // add to connection list  
  dst_id = dst.oid;
  
  sim.cllist($+1) = [1, 0, 0, src_port,   0, dst_id, dst_id, dst_port];
endfunction

// Connection to external outputs
function sim = libdyn_connect_extern_ou(sim, src, src_port, dst_port)
  libdyn_check_object(sim,src);
  
  if (src.objecttype == 4 | src.objecttype == 5) then // port specialiser bearbeiten FIXME: remove and chage the remainder of this fn as it is only a compatibility layer
   // FIXME REMOVED
   // printf("WARNING: libdyn_connect_extern_ou is not designed for handling port specialisers - your request\n");
   // printf("is redirected to libdyn_connect_outport, which you should use instead. src_port was ignored\n");
      // fixme: Aus Kompatiblitätsgrüden hier libdyn_connect_outport(sim, src, dst_out_port) aufrufen und src_port verwerfen

    // this recalls libdyn_connect_extern_ou ( this function )
    sim = libdyn_connect_outport(sim, src, dst_port);

  else // FIXME include if blk.objecttype == ??
     // A normal block
      
    if src.magic ~= 678234 then
      printf("bad block magic!");
      error("");
    end
   
    // test port sizes and types
    
    
    // check port sizes
    if (isfield(src, "outsizes") == %T) then
      // test for port sizes is possible -- otherwise the interpreter lib has to check

  //    printf("checking port sizes for outpus of the simulation\n");

      if src.outsizes(src_port+1) ~= sim.outsizes(dst_port+1) then
          printf("Error connecting port sizes %d --> %d\n", src.outsizes(src_port+1) , sim.outsizes(dst_port+1) );
          printf("Incorrect port sizes for simulation outputs");
          error("");
      end
    end    
  
  
    
    src_id = src.oid;
   
    //
    // matches 
    // if (cl[i].src_type == 0 && cl[i].dst_type == 1) { // connect block to external out
    // in libdyn.c: irpar_get_libdynconnlist
    //
    sim.cllist($+1) = [0, src_id, src_id, src_port,   1, 0, 0, dst_port];
  end
endfunction

function sim = libdyn_connect_inToou(sim, src_port, dst_port)
 
   
    // test port sizes and types
    
    
    // check port sizes


     if sim.insizes(src_port+1) ~= sim.outsizes(dst_port+1) then
         printf("Error connecting port sizes %d --> %d\n", sim.insizes(src_port+1) ,  sim.outsizes(dst_port+1) );
         printf("Incorrect port sizes for direktly connecting the simulation inputs to the outputs.");
         error("");
      end

  
  
    
    src_id = src.oid;
   
    //
    // matches 
    // if (cl[i].src_type == 1 && cl[i].dst_type == 1) { // connect ex in to ex out: not possible at the moment
    // in libdyn.c: irpar_get_libdynconnlist
    //
    sim.cllist($+1) = [1, 0, 0, src_port,   1, 0, 0, dst_port];


//   end
endfunction

// 
// Include a block that has no connections
// 
function sim = libdyn_include_block(sim, block)
  if block.magic ~= 678234 then
    printf("bad block magic!");
    error("");
  end
  libdyn_check_object(sim,block);
  
  
  // Add a special entry to the connectionlist
  src_id = 0; src_port = 0;
  dst_id = block.oid; dst_port = 0;
  
  sim.cllist($+1) = [0, src_id, src_id, src_port,   2, dst_id, dst_id, dst_port]; // 2 means a block without connections
endfunction


//
// Connection of port hint objects to external outputs high level
//
// src: port hint object
// dst_out_port: The output port to connect
//
function sim = libdyn_connect_outport(sim, src, dst_out_port)
      libdyn_check_object(sim,src);

        // connect a block to an output
        [sblk, sport] = libdyn_deref_porthint(sim, src);

 
// sblk.objecttype
// src.highlevelotype
        if sblk.objecttype == 1 then  // if src is a simulation input
          //printf("connect an input to an output\n");
          sim = libdyn_connect_inToou(sim, src_port=sport, dst_port=dst_out_port);
        end

        if sblk.objecttype == 0 then  // if src is a block within the simulation
          //printf("connect a block to an output\n");
          sim = libdyn_connect_extern_ou(sim, sblk, sport, dst_out_port);
        end
endfunction

//
// Higher Level
//


//
// Connect any object to inputs of blk object
//
// dblk is the destination block whose inputs are about to be connected
// with sources defined in input_list.
//
// input_list is the parameter list which contain:
// list(sblk, sport, ...) or list(sportHintObject, ...) or a mixture of both
//

// FIXME: Should be renamed to something like "libdyn_connect_inputs"
function [sim, output] = libdyn_conn_equation(sim, dblk, input_list)
// pause;
// printf("conn eq\n");
  if (length(input_list) == 0) // if there are no inputs given, then skip this function
//     printf("no inputs\n");
    output = dblk;
    
    return; // nothing to do
  end

  
 



  Nin_times2_ = length(input_list); // Anzahl Inputs in input_list * 2
  dport = 0; // start with first port
  
  i = 1;
  while i <= Nin_times2_ do // über alle Einträge in input_list
    
    // Teste ob ein object in input_list(i) liegt
    if (type(input_list(i)) == 17) then // FIXME Weiter überprüfen, ob es ein gültiger objecttype ist
      //printf("Ein object i=%d\n",i);
    else
      printf("Unexpected entry in input_list position %d (No libdyn object found)\n", i/2);      
      error("");
    end
    
    try
      sblk = input_list(i);
      libdyn_check_object(sim,sblk);
    catch
      printf("Unexpected entry in input_list position %d (No libdyn object found, or one that does not belong to the given simulation)\n", i/2);
      error("");    
    end

    
    // Check what kind of object sblk is
    CheckSuccess = 0;
    
    if (sblk.objecttype == 0 | sblk.objecttype == 1 | sblk.objecttype == 2) then
      sport = input_list(i+1); // hier müsste ein skalar (portnummer) anliegen
      if (type(sport) ~= 1) then
         printf("unexpected entry - I expected a port num\n");        
         error("");
      end    
      i=i+2; // Eins weiter in der parameter list; source portinformation überspringen
      
      CheckSuccess = 1;
    elseif (sblk.objecttype == 4 | sblk.objecttype == 5) then // port specialiser bearbeiten
      //printf("Ein port Hinweis object\n");
  
      // FIXME: Diese drei Zeilen in eigene function stecken:  [blk, port] = libdyn_dereference_obj(blk)    
//      highoid = sblk.highleveloid; // Hole object id des Referenzierten objects
//      sport = sblk.outport; // Hole die portnummer des Referenzierten objects
//      sblk = sim.objectlist(highoid); // Ersetze das Quell-Object
      [sblk, sport] = libdyn_deref_porthint(sim, sblk);
      
//      printf("Wurde auf oid %d und port %d dereferenziert\n", sblk.oid, sport);
      
      // schaue ob als nächstes vielleicht eine portnummer übergeben wurde (die eigendlich nicht notwendig ist)
      if (length(input_list) >= i+1) then
        check = input_list(i+1);
        if (type(check) == 1) then
// FIXME REMOVED
          //printf("WARNING: There was an additional port specifier given that is not needed because a port hint object was given\n");
          i=i+1; // Just ignore this additional information in the parameter list
        end    
      end

      
      i=i+1; // Springe weiter zu nächstem Argument in der Parameterliste
      
      CheckSuccess = 1;      
    end
    
    if (sblk.objecttype == 8) then // A feedback dummy referer as source
      // In this case no connection is set-up
      // Only the destination block and portnumber is stored in a data structure
      
      [sim] = libdyn_fill_feedback(sim, sblk, dblk, dport);
      CheckSuccess = 1;      

      // schaue ob als nächstes vielleicht eine portnummer übergeben wurde (die eigendlich nicht notwendig ist)
      if (length(input_list) >= i+1) then
        check = input_list(i+1);
        if (type(check) == 1) then
// FIXME REMOVED
        //  printf("WARNING: There was an additional port specifier given that is not needed because a feedback dummy was given\n");
          i=i+1; // Just ignore this additional information in the parameter list
        end    
      end


      i=i+1; // Springe weiter zu nächstem Argument in der Parameterliste
    end
    
    
    if CheckSuccess == 0 then
      printf("libdyn_conn_equation: unknown libdyn objecttype in input_list\n");
      error("");
    end

    
    // Jetzt steht "sblk" und "sport" zur Verfügung
        

    // Verbinde: Entweder zu Externem Input oder zu anderem Block; noop für feeback
    
    if sblk.objecttype == 0 then
      [sim] = libdyn_connect_block(sim, sblk, sport, dblk, dport);
    end
    if sblk.objecttype == 1 then
      [sim] = libdyn_connect_extern_in(sim, sport, dblk, dport);
    end
    if (sblk.objecttype == 8) then // A feedback dummy referer
      1; // noop  
    end
    

    
    dport = dport + 1;
  end
  
  
  output = dblk;
endfunction



//
// Virtual Superblock handling UNUSED maybe OBSOLETE and never tested
//

function [super_blk] = libdyn_new_super(sim, insizes, outsizes);
  super_blk.objecttype = 2; // Which object type is this? 2 - External super block
  super_blk.outsizes = outsizes;
  super_blk.insizes = insizes;
  
  super_blk.super_inobjs = list(); // Liste mit Blöcken + portnummer, die an die Superblock Inputs angeschlossen sind.
  super_blk.super_outobjs = list();   // Liste mit Blöcken + portnummer, die an die Superblock outputs angeschlossen sind.
endfunction

function [sim, super_blk] = libdyn_connect_super_in(sim, super_blk, port, dblk, dport)
  if super_blk.objecttype ~= 2 then 
    error("No super block!");
  end

  if dblk.objecttype ~= 0 then 
    error("Can not connect superblock input to other object than a normal block");
  end
  
  if (port < 0) | (port >= length(super_blk.insizes)) then
    error("incorrect port");
  end
  
  super_blk.super_inobjs(port*2 + 1) = dblk;
  super_blk.super_inobjs(port*2 + 2) = dport;
  
endfunction

function [sim, super_blk] = libdyn_connect_super_out(sim, sblk, sport, super_blk, port)
  if super_blk.objecttype ~= 2 then 
    error("No super block!");
  end

  if sblk.objecttype ~= 0 then 
    error("Can not connect superblock input to other object than a normal block");
  end
  
  if (port < 0) | (port >= length(super_blk.outsizes)) then
    error("incorrect port");
  end
  
  
  super_blk.super_outobjs(port*2 + 1) = sblk;
  super_blk.super_outobjs(port*2 + 2) = sport;  
endfunction



// Create new connection list for irpar structure
function sim = libdyn_build_cl(sim)
  cltype = 0;
  entries = length(sim.cllist);
  listelesize = 8;
  
  
  header = [cltype; entries; listelesize; 0]; // type, entries, listelesize, listofs
  listofs = length(header); // header len
  header(4) = listofs;
  
//    int dst_type = ret_.ipar_ptr[listofs + i * listelesize + 0]; // 0: normal block; 1: IN/OUT
//    int dst_blockid = ret_.ipar_ptr[listofs + i * listelesize + 1]; // blocks unique id
//    int dst_blockparid = ret_.ipar_ptr[listofs + i * listelesize + 2]; // blocks parameter id
//    int dst_port = ret_.ipar_ptr[listofs + i * listelesize + 3]; // which port of this dest block

  dsave = [];

  for clele = sim.cllist
    entry = clele(:);
    
    dsave = [dsave; entry];
  end
  
//  entry1 = [0; 1; 901; 0;   0; 2; 902; 0];
//  entry2 = [0; 2; 902; 0;   0; 3; 903; 0];

  //ges = [header; entry1; entry2];
  ges = [header; dsave];

  id = 100; // fixed irpar id for cl list
  sim.parlist = new_irparam_elemet(sim.parlist, id, IRPAR_LIBDYN_CONNLIST, [ges], []);
endfunction


//
// Set-up a new schematic which is defined by the function fn.
// insizes and outsizes contain one vector element for each in/outport port.
// The values of the element determines the size of the port respectively
//
// This is the main user function
//
function [sim_container_irpar, sim] = libdyn_setup_schematic(fn, insizes, outsizes);
    sim = libdyn_new_simulation(insizes, outsizes);

   [sim,simulation_inputs] = libdyn_get_external_ins(sim);
   
   // get all inputs
   inlist = list();
   for i = 1:length(insizes)
     [sim,key] = libdyn_new_oport_hint(sim, simulation_inputs, i-1);
     inlist(i) = key;
   end

   // let the user defined function describe the schematic
   // It will fill in sim
   [sim, outlist] = fn(sim, inlist);
   
   // check the number of provided outputs
   if (length(outsizes) > length(outlist)) then
      printf("libdyn: config for I/O: Your function provides %d outports and %d inports\n", length(outlist), length(inlist)  );
      printf("however, configured was\n");
      disp(outsizes); disp(insizes);
      error("libdyn: libdyn_setup_schematic: your function did not provide enough outputs");
   end
   
   if (length(outsizes) < length(outlist)) then
      printf("libdyn: config for I/O: Your function provides %d outports and %d inports\n", length(outlist), length(inlist)  );
      printf("however, configured was\n");
      disp(outsizes); disp(insizes);
      error("libdyn: libdyn_setup_schematic: your function provides too much outputs");
   end

   // connect outputs
   for i = 1:length(outsizes);
//       if exists(outlist(i)) then
//           1; // ok
//       else
//           error("libdyn: libdyn_setup_schematic: yout function did not provide enough outputs");
//       end
     sim = libdyn_connect_outport(sim, outlist(i), i-1);
   end
  
  
  
  // Collect and encode as irparam-set
  sim = libdyn_build_cl(sim); 
  sim_container_irpar = combine_irparam(sim.parlist);
  
//  // pack simulations into irpar container
//  parlist = new_irparam_elemet_box(parlist, sim_container_irpar.ipar, sim_container_irpar.rpar, sid);
endfunction

function [sim_container_irpar, sim, userdata] = libdyn_setup_sch2(fn, insizes, outsizes, intypes, outtypes, userdata);
// the same as libdyn_setup_schematic but userdata is feed to the schematic fn
// as well as returned and intypes and outtypes are now reserverd, but not used at the moment

// DEBUG
try
  if (ORTDdebug == 1) then
    printf("In libdyn_setup_sch2\n");
    pause;
  end
catch
 1;
end


   Ninports = length(insizes);
   Noutports = length(outsizes);

   if (Ninports ~= length(intypes)) then
     error("length of insizes and intypes not equal");
   end
   if (Noutports ~= length(outtypes)) then
     error("length of outsizes and outtypes not equal");
   end

   sim = libdyn_new_simulation2(insizes, outsizes, intypes, outtypes);

   [sim,simulation_inputs] = libdyn_get_external_ins(sim);   

   // get all inputs
   inlist = list();
   for i = 1:Ninports
     [sim,key] = libdyn_new_oport_hint(sim, simulation_inputs, i-1);
     inlist(i) = key;
   end

   // let the user defined function describe the schematic
   // It will fill in the structure "sim"

// TODO: Check inlist(i) types to match intypes

   [sim, outlist, userdata] = fn(sim, inlist, userdata);
//    printf("outlist:\n"); disp(outlist);

// TODO: Check outlist(i) types to match outtypes
   
   // check the number of provided outputs
   if (length(outsizes) > length(outlist)) then
      printf("libdyn: config for I/O: Your function provides %d outports and %d inports\n", length(outlist), length(inlist)  );
      printf("configured was\noutsizes:\n");
      disp(outsizes); printf("insizes: \n"); disp(insizes);
      error("libdyn: libdyn_setup_schematic: your function did not provide enough outputs");
   end
   
   if (length(outsizes) < length(outlist)) then
      printf("libdyn: config for I/O: Your function provides %d outports and %d inports\n", length(outlist), length(inlist)  );
      printf("configured was\noutsizes:\n");
      disp(outsizes); printf("insizes: \n"); disp(insizes);
      error("libdyn: libdyn_setup_schematic: your function provides too much outputs");
   end

   // connect outputs
   for i = 1:length(outsizes);

//      try

       sim = libdyn_connect_outport(sim, outlist(i), i-1);
       
//      catch
//        printf("Error while libdyn_connect_outport\n");        
//      end
   end
  
  
  
  // Collect and encode as irparam-set
  sim = libdyn_build_cl(sim); 
  sim_container_irpar = combine_irparam(sim.parlist);
  
//  // pack simulations into irpar container
//  parlist = new_irparam_elemet_box(parlist, sim_container_irpar.ipar, sim_container_irpar.rpar, sid);
endfunction












// 
// Signal collector framework WORK ONGOING
// 

function [sim] = ld_collect_signal_init(sim)
  sim.signal_collector = struct();
  sim.signal_collector.GroupNameList = [];
  sim.signal_collector.GroupStructs = list();
endfunction




function [sim] = ld_collect_signal(sim, group_ident_str, signal_name, in)

  GroupIndex = 0;

  N = length(sim.signal_collector.GroupNameList);

  for i=1:N-1
    printf(":: %d\n", i);
    str = sim.signal_collector.GroupNameList(i);

    printf(": %s\n", str);
    if str == group_ident_str then
      GroupIndex = i;
      printf("found %s\n", group_ident_str);
    end

  end

printf("ok\n");

  if ( GroupIndex == 0 ) then
    printf("%s was not found\n", group_ident_str);
    sim.signal_collector.GroupNameList = [ sim.signal_collector.GroupNameList; group_ident_str ];
  end



  // signal struct
  signal.name = signal_name;
  signal.libdyn_obj = in;

endfunction

//  [sim] = ld_collect_signal_init(sim)
// [sim] = ld_collect_signal(sim, group_ident_str="testgroup2", signal_name="cntrlvar", in="asignal")














function [sim] = ld_collect_signal__(sim, group_ident_str, signal_name, in)
  try
    getfield(group_ident_str, sim.signal_collector);
  catch
    group_list = list();

    // setfield
    cmd = sprintf("sim.signal_collector.%s = group_list", group_ident_str);
pause;
    eval(cmd);

//    error("ld_collect_signal: signal_name was not "
  end

  // signal struct
  signal.name = signal_name;
  signal.libdyn_obj = in;

  // get group list
  group_list = getfield(group_ident_str, sim.signal_collector);

  // insert into the group list
  i = length(group_list);
  group_list(i+1) = signal;

  // setfield
  cmd = sprintf("sim.signal_collector.%s = group_list", group_ident_str);
  eval(cmd);

endfunction


