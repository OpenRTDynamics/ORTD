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





//
// libdyn.sci - Scilab interface to libdyn
//
// 2010, 2011 Christian Klauer 
//
// depends on irpar.sci
//
//
//
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
ORTD.DATATYPE_INT = 2;
ORTD.DATATYPE_BOOLEAN = 3;
ORTD.DATATYPE_EVENT = 5;



// Globale variable which gives each simulation a unique and random id
// this id is stored in every object passed to the user, so further calls
// to this obj can check, wheter it belongs to the simulation passed
global libdyn_simu_id_counter;
libdyn_simu_id_counter = 1000 + ceil(rand()*10000);

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
  blk.oid = oid;
  blk.simid = sim.simid;
  blk.objecttype = 0; // Which object type is this? 0 - Block
  blk.input_block = 0;  // FIXME raus!
  blk.magic = 678234;
  blk.btype = btype;

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
  blk.oid = oid;
  blk.simid = sim.simid;
  blk.objecttype = 0; // Which object type is this? 0 - Block
  blk.input_block = 0;  // FIXME raus!
  blk.magic = 678234;
  blk.btype = btype;

  id = oid; // id for this parameter set - take the object id in this case as it is unique
  Nbipar = length(ipar);
  Nbrpar = length(rpar);
  eventlist_len = length(events);
  
  header = [ btype; oid; Nbipar; Nbrpar; eventlist_len ]; // add header parameters
  header = [ header; events(:) ]; // Add eventlist

  // set in and output sizes and types
  blk.insizes = insizes;
  blk.outsizes = outsizes;
  blk.intypes = intypes;
  blk.outtypes = outtypes;
  
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
    error("The given variable is no libdyn object");
  end

  if obj.simid ~= sim.simid then
    error("Object does not belong to this simulation");
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
  blk.oid = oid;
  blk.simid = sim.simid;

  blk.objecttype = 4; // Which object type is this? 4 - Special output port hint object
  blk.outsizes = [];
  
  // ---- special vars of this object class -----
  blk.highleveloid = object.oid;
  blk.outport = port;

  blk.magic = 678234;
  
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
  
//  printf("extrakt feedback dblk.oid=%d fbdummy.oid=%d\n", dblk.oid, oid);
endfunction

// Write information to a feedback dummy reference. The information contains the block
// plus port number of the destination of the feedback loop
function [sim] = libdyn_fill_feedback(sim, fbdummy, dblk, dport)
  libdyn_check_object(sim,fbdummy);
  
  oid = fbdummy.oid;
  
  tmp = sim.objectlist(oid);
  tmp.dblk = dblk;
  tmp.dport = dport;
  sim.objectlist(oid) = tmp;
endfunction


// Create new object, which refers to a certain output port of a block or an external input
// Mainly this is a reference to an higherlevel object + a given port number
// the returned object may also named "signal"
function [sim,fbdummy] = libdyn_new_feedback(sim);
  [sim,oid] = libdyn_new_objectid(sim); // get new object id
  fbdummy.oid = oid;
  fbdummy.simid = sim.simid;

  fbdummy.objecttype = 8; // Which object type is this? 8 - Special feedback dummy referer
  
  fbdummy.magic = 678234;

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
        error(".");
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
    error("bad block magic!");
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
             error("Incorrect port sizes");
      end
    catch 
        error("There was an error concerning the portsize checking. Maybe an interfacing function maldefined some input / output sizes\n");
    end
    
  end    

  if (isfield(src, "outtypes") == %T) & (isfield(dst, "intypes") == %T) then
      // test for port sizes is possible -- otherwise the interpreter lib has to check
      
  //    printf("checking port types\n");
      
      if src.outsizes(src_port+1) ~= dst.insizes(dst_port+1) then
         printf("Error connecting port types %d --> %d\n", src.outtypes(src_port+1) , dst.intypes(dst_port+1) );
             error("Incorrect port types");
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
    error("bad block magic!");
  end
  libdyn_check_object(sim,dst);


  // test port sizes and types
  if (isfield(dst, "insizes") == %T) then
      // test for port sizes is possible -- otherwise the interpreter lib has to check
      
//      printf("checking port sizes inputs of the simulation\n");
      
      if sim.insizes(src_port+1) ~= dst.insizes(dst_port+1) then
         printf("Error connecting port sizes %d --> %d\n", sim.insizes(src_port+1) , dst.insizes(dst_port+1) );
             error("Incorrect port sizes for simulation inputs");
      end
  end    


  // add to connection list  
  dst_id = dst.oid;
  
  sim.cllist($+1) = [1, 0, 0, src_port,   0, dst_id, dst_id, dst_port];
endfunction

// Connection to external outputs
function sim = libdyn_connect_extern_ou(sim, src, src_port, dst_port)
  libdyn_check_object(sim,src);
  
  if (src.objecttype == 4 | src.objecttype == 5) then // port specialiser bearbeiten FIXME: remove this as it is only a compatibility layer
   // FIXME REMOVED
   // printf("WARNING: libdyn_connect_extern_ou is not designed for handling port specialisers - your request\n");
   // printf("is redirected to libdyn_connect_outport, which you should use instead. src_port was ignored\n");
      // fixme: Aus Kompatiblitätsgrüden hier libdyn_connect_outport(sim, src, dst_out_port) aufrufen und src_port verwerfen
    sim = libdyn_connect_outport(sim, src, dst_port);
  else
     // A normal block
      
    if src.magic ~= 678234 then
      error("bad block magic!");
    end
   
    // test port sizes and types
    
    
    // check port sizes
    if (isfield(src, "outsizes") == %T) then
      // test for port sizes is possible -- otherwise the interpreter lib has to check

  //    printf("checking port sizes for outpus of the simulation\n");

      if src.outsizes(src_port+1) ~= sim.outsizes(dst_port+1) then
          printf("Error connecting port sizes %d --> %d\n", src.outsizes(src_port+1) , sim.outsizes(dst_port+1) );
          error("Incorrect port sizes for simulation outputs");
      end
    end    
  
  
    
    src_id = src.oid;
  
    sim.cllist($+1) = [0, src_id, src_id, src_port,   1, 0, 0, dst_port];
  end
endfunction

//
// Connection of port hint objects to external outputs high level
//
// src: port hint object
// dst_out_port: The output port to connect
//
function sim = libdyn_connect_outport(sim, src, dst_out_port)
      libdyn_check_object(sim,src);

      [sblk, sport] = libdyn_deref_porthint(sim, src);
      sim = libdyn_connect_extern_ou(sim, sblk, sport, dst_out_port);
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
function [sim, output] = libdyn_conn_equation(sim, dblk, input_list)
  Nin_times2_ = length(input_list); // Anzahl Inputs in input_list * 2
  dport = 0; // start with first port
  
  i = 1;
  while i <= Nin_times2_ do // über alle Einträge in input_list
    
    // Teste ob ein object in input_list(i) liegt
    if (type(input_list(i)) == 17) then // FIXME Weiter überprüfen, ob es ein gültiger objecttype ist
      //printf("Ein object i=%d\n",i);
    else
      error("unexpected entry in input_list (No libdyn object found)\n");
    end
    
    sblk = input_list(i);
    libdyn_check_object(sim,sblk);

    
    // Check what kind of object sblk is
    CheckSuccess = 0;
    
    if (sblk.objecttype == 0 | sblk.objecttype == 1 | sblk.objecttype == 2) then
      sport = input_list(i+1); // hier müsste ein skalar (portnummer) anliegen
      if (type(sport) ~= 1) then
        error("unexpected entry - I expected a port num\n");        
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
      error("libdyn_conn_equation: unknown libdyn objecttype in input_list\n");
    end

    
    // Jetzt steht "sblk" und "sport" zur Verfügung
    
    
//    // Untersuche, ob dblk oder sblk superblöcke sind, Wenn ja den dahintersteckenden Block auslesen
//    // und sblk oder dblk überschreiben (EXPERIMENTAL)
//    
//    if sblk.objecttype == 2 then // Quelle ist ein Superblock
//      new_sblk = sblk.super_outobjs(sport*2 + 1); // Lese den Block der für den Superblockausgang verantwortlich ist
//      new_sport = sblk.super_outobjs(sport*2 + 2);
//      
//      printf("Substituted a superblock out with block %d\n", new_sblk.oid);
//      
//      sblk = new_sblk;
//      sport = new_sport;
//    end
//    
//    if dblk.objecttype == 2 then // Ziel ist ein Superblock
//      new_dblk = dblk.super_inobjs(dport*2 + 1); // Lese den Block der für den Superblockeingang verantwortlich ist
//      new_dport = dblk.super_inobjs(dport*2 + 2);
//
//      printf("Substituted a superblock in with block %d\n", new_sblk.oid);
//
//      dblk = new_dblk;
//      dport = new_dport;
//    end

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
      error("libdyn: libdyn_setup_schematic: your function did not provide enough outputs");
   end
   
   if (length(outsizes) < length(outlist)) then
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

   Ninports = length(insizes);
   Noutports = length(outsizes);

   if (Ninports ~= length(intypes)) then
     error("length of insizes and intypes not equal");
   end
   if (Noutports ~= length(outtypes)) then
     error("length of outsizes and outtypes not equal");
   end

    sim = libdyn_new_simulation(insizes, outsizes);

   [sim,simulation_inputs] = libdyn_get_external_ins(sim);   

   // get all inputs
   inlist = list();
   for i = 1:Ninports
     [sim,key] = libdyn_new_oport_hint(sim, simulation_inputs, i-1);
     inlist(i) = key;
   end

   // let the user defined function describe the schematic
   // It will fill in sim

   [sim, outlist, userdata] = fn(sim, inlist, userdata);
   
   // check the number of provided outputs
   if (length(outsizes) > length(outlist)) then
      error("libdyn: libdyn_setup_schematic: your function did not provide enough outputs");
   end
   
   if (length(outsizes) < length(outlist)) then
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




//////////////////////////////////////////////////////////
// More complex blocks based on elementary blocks
//////////////////////////////////////////////////////////

//
// Generic controllers
//

function [sim,u] = ld_standard_controller(sim, event, r, y, K)
// classic linear controller
    [sim,e] = ld_sum(sim, event, list(r,0, y,0), 1,-1 );
    [sim,u] = ld_ztf(sim, event, list(e,0), K);
endfunction

function [sim,u] = ld_standard_controller_2dof(sim, event, r, y, K, M)
// like classic controller but with a measurement filter M
    [sim,y_] = ld_ztf(sim, event, list(y,0), M);
    [sim,e] = ld_sum(sim, event, list(r,0, y_,0), 1,-1 );
    [sim,u] = ld_ztf(sim, event, list(e,0), K);
endfunction

function [sim,u,u_fb,u_ff] = ld_standard_controller_ffw(sim, event, r, y, K, Gm1, T)
// controller with a feedforwad part
    [sim,r_] = ld_ztf(sim, event, list(r,0), T); // closed loop model
    
    [sim,e] = ld_sum(sim, event, list(r_,0, y,0), 1,-1 );
    [sim,u_fb] = ld_ztf(sim, event, list(e,0), K);
    
    [sim,u_ff] = ld_ztf(sim, event, list(r,0), Gm1);
    [sim,u] = ld_sum(sim, event, list(u_fb,0, u_ff,0), 1,1 );
endfunction



