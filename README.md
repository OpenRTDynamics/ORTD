
OpenRTDynamics v1.00-svn -- Open Real-Time Dynamics - A framework
                            for implementing real-time controllers
                            and signal processing algorithms.

                            (openrtdynamics.github.io)

Data of this document: 18.7.2018

The Real-Time Dynamics Framework is a novel approach to the implementation
of block- / signal-based schematics, commonly used in control engineering 
and signal processing, for real-time usage (like Real-Time Workshop® or 
Scicos Code Generation). This framework is suitable for time discrete signal 
processing algorithms, using the same principles like in Scicos/Xcos or 
Simulink. However, schematics are described by combining special Scilab 
functions provided by the framework.

In contrast to Xcos or Simulink, the framework does not require source 
code generation / compilation steps to create rt-programs and therefore 
does not rely on C-compilers for the target system that had to be installed 
on the development computer. Instead an efficient to online-interprete 
binary representation is assembled by the framework-provided Scilab functions
and executed by the libortd real-time interpreter.

Due to the description using Scilab, well structured code can be achieved, 
whereby i.e. a filter design could be placed nearby the implementation 
for a better human-readability. Additionally, functional and object orientated 
design schemes can be realised. The latter feature enables the possibility to 
implement reusable parametrised sub-schematics that may also change in its 
structure based on parameters.

Further, along with the remote control interface and the ability to 
implement state machines, to run different simulations in threads (e.g. 
to distribute calculations across multiple CPUs) and to include Scilab-Code 
as computational functions (S-function like), ORTD is also ideally suited 
for laboratory automation. The ability to replace sub-schematics with new 
implementations and parameters during runtime removes the effort of 
restarting the main real-time program. Simulations running in threads can 
also be synchronised to sensor updates or other events e.g. variable timers. 

Please Note: OpenRTDynamics does not depend on any part of Scicos/Xcos.
             ORTD uses algorithms for time-deterministic interpretation of 
             simulations.
             Only the Scilab Scripting language is required for describing
             the schematics and only during development. During runtime, no
             more than standard C/C++ libraries are required, therefore
             ORTD is very uncomplicated to install and maintain, even on
             other platforms. The need for difficult and cryptic instructions 
             for the installation of this framework was intentionally avoided.
             

The ORTD runtime library can be easily integrated into other simulation 
frameworks e.g. Scicos or used within other software by including a shared 
library.


IMPORTANT NEWS and NOTES
========================

- Please note: Scilab 6 and above does not work so far as the syntax slightly changed 
  breaking the compatibility. Please use one the older 5.x versions. 

- Rev 592: Full support for PaPI (Graphical interface to control targets)

- Rev 519: A new command "ortdrun" has been introduced that starts the 
  ortd-interpreter. The old command ortd becomes obsolete. However, most
  examples still use the old command in their shell scripts.

- Rev 495: A new framework for remote control / communication has been integrated
  called Packet Framework. This renders the old rt_server module obsolete (c.f. Sec.
  REMOTE CONTROL INTERFACE). Build e.g. web interfaces to your ORTD implementation!
  The command line option --master_tcpport can (and should) be omitted when calling
  the command ortd, if rt_server is not used.

- Though it is used in some examples, do not use the block "ld_simnest" 
  for running asynchronous simulations any more. These not updated examples 
  may not terminate on Cntrl-C properly. Use ld_async_simulation instead. 

[[project_screenshots]]


--- The features that make this framework powerful ---

* Nesting simulations (sub-controllers) into each other up to unlimited depth,
  using state machines (each state is represented by a simulation that is
  reset when the state is left), or asynchronously running (threaded) sub-
  simulations. Sub-simulations are included by special blocks that
  implement a framework for state machines or threads and take functions 
  describing the sub-simulations as their arguments along with their I/O 
  ports. (modules/nested)

* Ability to synchronise threaded sub-simulations to external event sources,
  e.g. timers (modules/synchronisation), incoming network packages or 
  sensor readings...

* Online replacement of sub-controllers; Yes, can exchange wires and blocks
  during the controller is running (modules/nested)

* Communication with Scilab from the simulations (modules/scilab),
  e.g. for running calibration scripts on previously recorded data

--- Other nice features ---

* Mathematical formula parsing (modules/muparser)
* Remote control interface to e.g. buid a webinterface (examples/PacketFramework)
* Starting external processes (modules/ext_process)
* Ability to call time-discrete Xcos-Blocks trough a wrapper (modules/scicos_blocks), 
  e.g. those from the HART toolbox. If you're interested, please ask me how to do so,
  as I didn't publish some code that would automate this by now.
* Networking: raw UDP send- and receive functionality
* ...




        
CONTENTS OF THIS README
=======================

  PART  I  --  TECHNICAL INFORMATION

- OS DEPENDENCY
- LICENSE
- REQUIREMENTS
- INSTALLATION (GRAPHICAL INSTALLATION)
- GETTING STARTED
- COMPILATION / INSTALLATION (normal PC x86/amd64)
- IMPORTANT MAKE-TARGETS
- BUILDING A SELF-EXTRACTING AND COMPILING PACKAGE
- UPDATING THE SUBVERSION VERSION
- INSTALLATION ON BEAGLEBONE (and likely other Boards that support Ubuntu for ARM)
- INCLUDING YOUR HARDWARE VIA PLUGINS OR MODULES
- TARGET CONFIGURATION (for usage in a hard real-time environment)
- ANDROID (ANDROID_ARM-target)
- CALLING SCILAB FROM ORTD SIMULATIONS
- HART-Module INSTALLATION
- TOOLS
- LIST OF PROVIDED BLOCKS
- MODULES
- DEVELOPING MODULES & BLOCKS
- ADDITIONAL THINGS
- PRINCIPLES FOR DEVELOPING SCHEMATICS
- EXAMPLES
- DATATYPES
- NOTES ON REAL-TIME CAPABILITIES
- REMOTE CONTROL INTERFACE (PaPi)
- IMPORTANT SOURCE FILES
- REAL-TIME BENCHMARK
- PORTING TO OTHER TARGETS

  PART  II  --  BACKGROUND INFORMATION

- MOTIVATION FOR ANOTHER REAL-TIME FRAMEWORK

  APPENDIX  --  GOOD TO KNOW

- CONTRIBUTIONS
- TODO
- BUGS
- CONTACT
- CHANGELOG






PART  I  --  TECHNICAL INFORMATION
- - - - - - - - - - - - - - - - - - - -


                         
OS DEPENDENCY
=============

- Runs on Linux. 
- Also compiles & runs on the armv7l / Linux platform (Beaglebone, Rasperry PI and 
                                                                        many others)
- And probably other UNIX-like OS but never tried

On Linux one of the rare APIs that stayed (almost) compatible for more than several
year are the ones of libc, pthreads, and the standard C++ - libraries :-) and :-(, as 
there could be more constant APIs. Since OpenRTDynamics is supposed to also stay 
functional in the long term future, this framework is *only* depending these very 
fundamental libraries. However, the functionally can be enhanced by additional libraries 
that are automatically detected and used.

Because of these relaxed dependencies, it should not be a big issue to port
this framework to other (real-time) OSs.

LICENSE
=======

LGPL, see COPYING / COPYING.LESSER

REQUIREMENTS
============

On the development host:
 * Linux (Ubuntu is tested)
 * Scilab 5.3. or above. It will be automatically installed via the graphical installer.
 * Please note: Scilab 6 and above does not work so far as the syntax slightly changed breaking the compatibility.

On the executing / real-time target (can also be the development host):
 * Linux (Ubuntu is tested, rt_preemption is supported but not required)
 * Nothing more than standard libraries (libc, libpthreads, C++ standard libs, ...)
 * Optional: An installation of Scilab, libblas, libgsl

For compiling the framework
 * Standard tools like: bash, gcc, g++, make, perl

 Optional libraries
 * libgsl, libblas (If their corresponding development packages are installed, some
   additional functionality is gained. They are automatically detected when running
   make config)

INSTALLATION (GRAPHICAL INSTALLATION)
=====================================

Install required packets (Ubuntu): 

    sudo apt-get install zenity g++ make subversion build-essential

Download and Install Scilab from http://www.scilab.org

Download the latest packet from http://sourceforge.net/projects/openrtdynamics/files/
and unpack into the directory you like to install ORTD (may be the home directory)

Go into the extracted folder and run

% sh setup.sh

Alternatively if no graphical environment is available, e.g. on embedded systems

$ make config
$ make install

In this case only the interpreter is installed but not the Scilab-Toolbox for
ORTD-development.

GETTING STARTED
===============

Read section PRINCIPLES FOR DEVELOPING SCHEMATICS to learn the basics of ORTD-coding
and have a look at the huge amount of examples.

Consider the files in examples/Template_rtcontrol/ as a starting point for setting up
realtime control applications.

COMPILATION / INSTALLATION (normal PC x86/amd64)
================================================

The suggested way is to use the automatic installation routine as described on their
website. However, if you intent to do the compilation / installation by hand you can go 
on with the following procedure:

- Configure your target system through target.conf (not needed for normal installations).
- Run "make config" and then "make" followed by "make install" on x32/x64/ARM-Linux platforms.

This will compile libortd.so and bin/ortd the generic 
schematic interpreter. Both files are copied to /usr/local/[bin,lib] respectively.


The command

     $ make install_toolbox

will ask you for your Scilab 5 installation directory (where the package from the
Scilab Website was extracted) and installs the Scilab toolbox.

The manual way woulf be to link the "ld_toolbox" directory (not only its contents) 
to your <scilab installation directory>/share/scilab/contrib .

Use

    $ make scilabhelp

to generate help files for the Scilab help. (Try e.g. "help ld_gain" in Scilab to test it)

Cleaning up is done via

    $ make clean

This should be executed when compiling and updated version from subversion.

To visualise data it is suggested to additionally install PaPi 
(https://github.com/TUB-Control/PaPI).


IMPORTANT MAKE-TARGETS
======================

    make config          # Configure the framework, e.g. check for libraries
    make                 # Compile
    make install         # Install to /usr/local/[lib,bin]
    make homeinstall     # Install the ortd binary into the home directory ( ~/bin )
    make demo            # run a demo
    make package         # Create a self extracting and self compiling copy of the framework 
			  contained within a shell archieve OpenRTDynamics_source.sh
			  which can be transfered to target platforms e.g. ARM-targets
			  like the Beaglebone, Rasperry Pi, ...
			  The archieve requires a build system on the target system.

    make scilabhelp      # Update the help of the Scilab Toolbox
    make install_toolbox # Run the installer for the Scilab Toolbox
    make clean           # Clean up, always use this after updating the framework e.g. via svn
    make cleanBuildFiles # remove all files generated during the build process, but not the
			   resulting binaries
    make clearconfig     # Delete the information gathered during make config.
    make update          # Download the latest version from the subversion repository and complile it

BUILDING A SELF-EXTRACTING AND COMPILING PACKAGE
================================================

To build a self extracting and self compiling archieve run:

    $ bash <ORTDFolder>/AutoBuild/LINUX_source/build.sh this <openrtdynamics-source> <OpenRTDynamics_source>

from outside of the openrtdynamics-source directory or alternatively

    $ make package

from within the source tree.

This gives a file called <OpenRTDynamics_source>.sh which inclues a copy of the cleaned up folder 
<openrtdynamics-source>. This can then be transfered to target systems, where the 
sources are compiled automatically. Of course a build system is required on the target system.

UPDATING THE LATEST SUBVERSION VERSION
======================================

To enable the latest features:

Use 

    make update ; make install ; make homeinstall

to get a correct update. Remember to re-compile your plugings, if any.


INSTALLATION ON EMBEDDED (ARM)-Systems 
======================================



Quick Installation on target system
---------------------------------------

Testet for Ubuntu 12.04, but should also work for Ångström-Linux.

On Ubuntu you could also install Scilab before (to enable Scilab embedded into ORTD): 

    sudo apt-get install scilab

Then proceed using:

1. Make sure you have the gnu compilers, perl and make on your target system (should be the case)
2. Download the default package from <http://sourceforge.net/projects/openrtdynamics/files/> to the target
   device.
3. Install using "make config" & "make install"


Hints for development
---------------------

In order to develop schematics ,you can prepare them on your PC using Scilab and the 
installed Toolbox and upload the *.[i,r]par files to your Beaglebone, where they get 
executed. For this it is suggested to use some file transfer protocol like SFTP to 
upload files. You can install the ssh-server on your Beaglebone using

    $ sudo apt-get install openssh-server .

Then you can connect via Gnome "Connect to server" or whatever you like. For directly 
storing your whole project on the Beaglebone Filesystem you can also investigate 
the "~/.gvfs" folder and use it to edit / execute Scilab files directly on your PC, 
whereby the files are stored on your Beaglebone. By this you would not copy the 
irpar-files by hand every time. You also need an installation of ORTD on your host
for development using Scilab (see above).

Example application
-------------------

An example for accessing the BB hardware, is available in examples/beaglebone. 
To run it, do on the BB:

    $ sh build.sh
    $ sudo sh run.sh

The LED should now blink randomly.



INCLUDING YOUR HARDWARE VIA PLUGINS OR MODULES
==============================================

Hardware can be added via new blocks by the plugin interface or by adding your own 
module in the modules/ folder or by plugins (shared libraries). 

See also section -- DEVELOPING MODULES & BLOCKS -- for an example.

Old information:
  For an example on plugins, please have a look at examples/plugin_example. For using
  plugins you need have the Target "Linux" enabled (see TARGET CONFIGURATION above).


TARGET CONFIGURATION (to adjust the target system for the interpreter)
======================================================================

The following targets can be choosen in target.conf:

* LINUX (Default, includes support for RT-Preemption if available)
* MACOSX Uses the timers and real-time support of the Mach kernel as present in Mac os X

* LINUX_DEBUG   (Like LINUX but debugger information is included)
* CYGWIN (not functional up to now, to run ORTD-simulations on Windows)
* ANDROID_ARM (Android using Android NDK cross compile toolchain, see the android section in
                this document)
* ANDROID_ARM_NEON (Like above but with NEON-support for newer arm devices)
* LINUX_x86_32 Generates a 32Bit-build of openrtdynamics on an x86_64 Linux operating system.
                On Ubuntu installing "gcc-multilib", "ia32-libs" and "g++-multilib" is required.
* XCOS_RTAI_COMPATIBLE For including ORTD into Scicos/Xcos. Possibly including RTAI-Code generation.
* RTAI_COMPATIBLE (runs within RTAI; Plugins are disabled because shared libraries
                    are not supported in RTAI, NOTE: This has not been tested for a long time)
       
INSTALLATION ON MACOS X
=======================

Note: Currently the graphical installer does not work on Mac. You need to do the manual installation:

Optional: Install homebrew, then: brew install gsl

0) Configure the ortd-target

     $ echo "MACOSX" > target.conf

1) Install Scilab such that it will be stored in /Applications
2) make config (this will check e.g. for libgsl and libblas)
3) make install_toolbox (answer to the questions; The corresponding Scilab-binary will be used by the ortd-module "scilab" )
4) make ; make install
5) make scilabhelp

Note: On Mac, only the new interpreter command "ortdrun" is supported but the shell scripts (*.sh) in most examples still use the 
old command "ortd" instead!





0) If you like, install Scilab and create a link named "scilab" that is e.g. stored in /usr/local/bin
   to the binary of Scilab, such that a command "scilab" is available in the command line.
   e.g.:
   
    $ ln -s /Applications/scilab-5.5.0.app/Contents/MacOS/bin/scilab /usr/local/bin/scilab 

In the main directory of ORTD do:

    2) $ make config
    3) $ make ; make install

Create a link in the subfolder share/scilab/contrib/ of the Scilab installation that points to 
the subfolder scilab/ld_toolbox of ORTD, e.g.:

    ln -s /Users/chr/svn/openrtdynamics/trunk/scilab/ld_toolbox  /Applications/scilab-5.5.0.app/Contents/MacOS/share/scilab/contrib
    

                
ANDROID (ANDROID_ARM-target)
============================

PLEASE NOTE: Currently Android-Support is broken. Small changes to the file realtime.c are
             required, but this is not in the focus of development currently...

Requires the executables from the NDK (<http://developer.android.com/tools/sdk/ndk/index.html>):

     arm-linux-androideabi-gcc,    arm-linux-androideabi-c++,   arm-linux-androideabi-g++
   
Therefore, let the NDK create a toolchain for you and set the environment variables
(as described in docs/STANDALONE-TOOLCHAIN.HTML of the NDK's documentation folder) 

     export PATH=<Path to AndroidArmToolchain>/bin/:$PATH

currently there is only the ortd & ortd_static executable available,
which can be run from e.g. an android terminal application. (No nice App for now)

NOTE: at the moment, before compiling through "make" you need to remove the EDF-module
      as it does not compile for android:   rm -rf modules/EDF


PRECOMPILED BINARIES
--------------------

A not so recently updated and compiled versions are available from:

http://openrtdynamics.sf.net/download/Android/current/OpenRTDynamics_android.tgz
 
Only the static executable can be obtained from:

http://openrtdynamics.sf.net/download/Android/current/ortd_static.gz

You can run the binary using an android terminal emulator like e.g.
terminalIDE if you copy the executable "ortd_static" to a folder "bin"
in the home directory. Also create the links

     ln -s ortd_static ortd
     ln -s ortd_static libdyn_generic_exec


Using with App TerminalIDE
--------------------------

NOTE: The following is a work in progress and does not work
      for now, because hostnames are not resolved.

When you use the Terminal-App "terminalIDE" you can copy and paste the
following lines to install the pre-compiled framework:


    ## Just install the executable
    cd
    mkdir bin
    cd bin
    rm -f ortd_static ortd 
    rm -f libdyn_generic_exec_static 
    rm -f libdyn_generic_exec

    wget http://openrtdynamics.sf.net/download/Android/current/ortd_static.gz
    gzip -d ortd_static.gz
    chmod +x ortd_static

    ln -s ortd_static ortd
    ln -s ortd_static libdyn_generic_exec
    ln -s ortd_static libdyn_generic_exec_static


    ## Install the whole compiled framework including the examples
    cd
    wget http://openrtdynamics.sf.net/download/Android/current/OpenRTDynamics_android.tgz
    tar xfvz OpenRTDynamics_android.tgz
    rm OpenRTDynamics_android.tgz

    #
    # OR simpler
    #
    wget http://openrtdynamics.sf.net/getitAndroid.sh -O - | bash


CALLING SCILAB FROM ORTD SIMULATIONS
====================================

With help of the module "scilab" (modules/scilab) blocks can be implemented
using the scilab language (example available in (modules/scilab/demo/ScilabRTTest).
For this, Scilab is started as a separate process and interfaced by
standard input / output streams.

A built-in path for the Scilab executable is chosen during compilation
of the library (i.e. make) as follows:

1) The scilab executable from the Scilab installation that comes along the graphical
   installation of ORTD is used (configured in the file scilabdir.conf).

2) If there is a command "scilab" in the system this one is used if 1)
   was not successful.

3) No built-in path for the Scilab binary is included. However, the binary
   can be specified in the block "ld_scilab*" that includes the Scilab code.

HART-Module INSTALLATION
========================

Please Note: The hart module only supports Scilab 4.1.2. For newer Scilab
             versions the module has to be adapted a bit. 

Compile & install the framework using the target "XCOS_RTAI_COMPATIBLE":

    echo "XCOS_RTAI_COMPATIBLE" > target.conf
    make clean; make; make install

Link hart/hart_ortd to the modules/ directory of your HART-Toolbox. 

    ln -s hart/hart_ortd <hart-dir>/modules

WARNING: Do not make a copy -- otherwise some header files are not found
         during compilation.

TOOLS
=====

- ortdrun, ortdrun_static: ORTD-interpreter to run realtime code.

Depreciated (do not use when possible):

- ortd: (the new name of libdyn_generic_exec)
    executes schematics in real-time (rt_preempt) OR as fast as possible
    Schematics are getting some dummy in and outputs, but should not be
    used.

    type "ortd --help" for help

- ortd_static: The statically linked version of the executable
               (can be used without installing libraries into 
                the system, runs nearly everywhere)

LIST OF PROVIDED BLOCKS
=======================

Have a look at documentation/List_of_Module_Blocks.txt.
It is an automatically generated list of available blocks.

In Scilab try "help ld_gain" to get into the help folders for ORTD.

MODULES
=======

Modules are placed within the modules subdirectory. For now, you will find the
following:

- muparser: Evaluation of mathematical expressions
- udp_communication / PacketFramework: Remote control of your controller via an UDP-Interface
- nested: includind and switching of ORTD-Simulations within ORTD-Simulations
- basic_ldblocks: Basic Blocks (hysteresis, modulo counter, (de)multiplexer, 2to1 switch ...)
- template: A module template for creating your own blocks
- ...
- ... (Need to update this list)

Embedded Scilab
----------------

During compilation, the ortd-Makefiles search for a Scilab-binary in the following order:

1) use the binary provided by the path in "scilabdir.conf", that was generated during make install_toolbox
2) Check for a command "scilab"

To check which Scilab-binary is actually used by the ortd interpreter:

    $ cat modules/scilab/scilabconf.h


DEVELOPING MODULES & BLOCKS
===========================

There is a template in examples/TemplateModule_V2.
An older version is found in modules/template".
For very basic block functionality you can also refer to modules/basic_ldblocks

IMPORTANT: Since there is no guarantee for a stable ABI (Application binary interface)
           you should *re-compile* any ORTD-plugin after updating ORTD. The API
           (defined by the ORTD-header files) however is going to stay compatible,
           so you do not have to change you source code for each new version of ORTD,
           which probably makes your life easier :-)

ADDITIONAL THINGS
=================

Within additional/joystick, there is a separate Python-Script to read out a connected
joystick and feed the data directly to ORTD-schematics via the rt_server infrastructure.
It can be used for an example to easily integrate other input devices. Only one simple 
Python-Script is needed for that.

PRINCIPLES FOR DEVELOPING SCHEMATICS
====================================

Each block is represented by a call to a scilab function that includes a block
or a set of blocks into a simulation:

    [sim, u] = ld_play_simple(sim, ev, r=[1,2,3,4,5,4,3,2,1] );
    [sim, y] = ld_gain(sim, ev, u, 1.5);

The description of the simulation is stored within the structure "sim". The variables
"u" and "y" represent in- and output signals that are used to connect blocks. The prefix 
"ld_" denotes "lib dynamic", which is the name of the interpreter. The parameter "ev" 
is due to a design decision in the past that was then never used. It must be set to zero 
(ev=0) at the moment. In a next major revision this is likely that there will be new 
block interfaces where this will be removed. In this simple example, during each 
simulation time step k, the current value u[k]=r[k] is multiplied by 1.5 yielding 
y[k] = 1.5 * r[k].

It is important to note, that not the scilab commands are executed during runtime. They
are only used to describe the blocks and their connections. Functions with the ld_-prefix
are typically defined in the scilab_loader.sce files.

Blocks that synchronise simulations
-----------------------------------

The OpenRTDynamics simulator provides a special callback-based API that allows one
block in a simulation running in a thread to trigger simulation steps.
This is used by some special blocks to trigger a simulation step of the simulation containing
such a block. They are used to synchronise the simulation when e.g. an event occurs, e.g. when
a network packet arrives, a certain amount of time has passed or the thread was signaled by
another one or new sensor data is available (and more). Some of these blocks are:
  
  * ld_ClockSync          // Sync to a timer
  * ld_AndroidSensors     // When a measurement becomes available
                          // one simulation step is performed (only Android)
  * ld_UDPSocket_Recv     // One simulation when a network packet arrives
  * ld_RecvNotifications  // Receives notifications triigered by other simulations
                          // running in other threads


Such blocks must be placed into simulations that are running in threads started by 
"ld_async_simulation".

Examples are e.g. given in

    modules/synchronisation/demo/timed_thread_v2.sce
    modules/udp_communication/demo/udp_receiver.sce

An template of the C++ - source code for such a block is given in 

    examples/TemplateModule_V2/Template_blocks.cpp, class SynchronisingTemplateBlock



Threads
-------

Threads are initiated by

        ThreadPrioStruct.prio1=ORTD.ORTD_RT_NORMALTASK; // or  ORTD.ORTD_RT_NORMALTASK
        ThreadPrioStruct.prio2=0; // for ORTD.ORTD_RT_REALTIMETASK: 1-99 as described in   man sched_setscheduler
        ThreadPrioStruct.cpu = 1; // -1 means dynamically assign CPU

        [sim, startcalc] = ld_initimpuls(sim, 0); // triggers your computation only once

        [sim, outlist, computation_finished] = ld_async_simulation(sim, 0, ...
                              inlist=list(), ...
                              insizes=[], outsizes=[], ...
                              intypes=[], outtypes=[], ...
                              nested_fn = Threadfn, ...
                              TriggerSignal=startcalc, name="VisThread", ...
                              ThreadPrioStruct, userdata=list() );

hereby the time-scheduling parameters like the thread's priority or CPU affinity can be 
configured. The ORTD-Superblock

	function [sim, outlist, userdata] = Threadfn(sim, inlist, userdata)
	  // This sub-simulation will run in a thread
          // wait for event
          [sim, signal ] = ld_RecvNotifications(sim, 0, ObjectIdentifyer="signal");
          [sim, signal___] = ld_Int32ToFloat(sim, ev, signal ); // signal must be catched, otherwise ld_RecvNotifications will be optimised out

	  [sim, R] = ld_read_global_memory(sim, ev, index=readI, ident_str="RotMat", datatype=ORTD.DATATYPE_FLOAT, ElementsToRead=9);
          [sim] = ld_printf(sim, ev, R,   "R = ", 9);

	  // output of schematic
	  outlist = list();
	endfunction

will then run in thread and the simulation is synchronised using ld_RecvNotifications to the
occurrences of events sent to this thread (c.f. section "Blocks that synchronise simulations").

EXAMPLES
========

Type "make demo" in the main folder.

Examples can be found within the "examples" subdirectory.

FIXME: Update this

For a ready to run example go into the examples directory via shell and type:

    $ sh sh_runoscillator

A second try could be to execute "simple_demo.sce" within scilab 5 or above. 
It will create *.[i,r]par files containing the schematic.
Ensure that you loaded the "ld_toolbox" at first. Then run the command 

    $ ortd -s simple_demo -i 901 -l 100

within the "examples" directory. It will load the [i,r]par files, 
compile the schematic and start execution. This example will create
several “.dat” output files.

You can also use simple_demo.sce as a template.

DATATYPES
=========

- ORTD.DATATYPE_FLOAT (Scilab), DATATYPE_FLOAT (C) is "double"
- ORTD.DATATYPE_INT32 (Scilab), DATATYPE_INT32 (C) is "int32_t"
- ORTD.DATATYPE_BINARY (Scilab), DATATYPE_BINARY (C) is "char" (8 Bit)

- more to come ...

Nevertheless the majority of the available blocks *only* support
DATATYPE_FLOAT by now. Especially the ld_statemachine block
does not support datatypes other than DATATYPE_FLOAT.
A block ld_statemachine2 is about to come...

For data conversion the following blocks are available up to now:

Simple datatypes: ld_roundInt32, ld_ceilInt32, ld_floorInt32, ld_Int32ToFloat
Structures: ld_DisassembleData, ld_ConcateData

NOTES ON REAL-TIME CAPABILITIES
===============================

1) Please note that there are serveral blocks that call I/O operations
of the OS. Among theses are ld_printf, ld_ReadAsciiFile, ld_UDPSocket_SendTo
and many others. Such blocks can potentially disturbe the real-time execution
by introducing unpredictable delays.
If low jitter is allowed, you may consider using such blocks in separate 
simulations running in separated thread that communicate to the hard real-time
simulation using rinfbuffers (ld_ringbuf) or shared memory (ld_global_memory).
However, the blocks ld_savefile and ld_file_save_machine are suitable for running
in real-time simulations to save data to disk, because they internally use a
ringbuffer to communicate to a separated thread for calling the OS.

2) The execution time of one simulation step can be measured using ld_clock that
reads the clock of the computer.

3) Apply the proper priorities to the threads. It is suggested to start with
the template examples/Template_rtcontrol.

REMOTE CONTROL INTERFACE (e.g. using PaPi)
==========================================

An UDP-based communication protocoll (PacketFramework) is available that can be used to 
visualise data e.g. using PaPi (https://github.com/TUB-Control/PaPI).

Examples for using PaPi to display automatic calibration procedures:

   https://github.com/christianausb/OpenRTDynamics/blob/master/Examples/PaPi/AutomaticProcedures

and

  https://github.com/TUB-Control/PaPI/tree/development/data_sources/ORTD/DataSourceChangingAutoConfigExample

How it looks like:

[[embed url=http://www.youtube.com/watch?v=9B2BISXaPdo]]

An example that visualises data using a web-browser is available at

   https://github.com/christianausb/OpenRTDynamics/tree/master/Examples/nodejs_webinterface_Level2
   or examples/PacketFramework

How it looks like:

[[embed url=http://www.youtube.com/watch?v=Mln_JxfzLD0]]


OBSOLETE: A TCP-based remote control interface is provided by the rt_server module, providing
ld_parameter and ld_stream. As it is thought about a more simple (in terms of lines
of C-code) (UDP)packet-based communication in combination with an external javascript 
(node.js) program for higher level logic, the infrastructure will not be further 
developed. 



IMPORTANT SOURCE FILES
======================

- modules/basic_ldblocks/[basic_ldblocks.c,basicCppBlocks.cpp]:
               Most blocks for basic functionality. Could be also
               be used as templates / examples for block computational
               functions.

- modules/basic_ldblocks/scilab_loader.sce
               Interfacing functions for most blocks for basic 
               functionality. Could be also be used as templates /
               examples for block computational functions.

- irpar.[c,h]: A small and efficient implementation for storing
               vectors of data, strings or other objects within
               a vector of "double" and another containing "int"
               values.

- libdyn.[c,h]: The core simulator implementation, which reads the
                list of block interconnections, sets up all blocks,
                determines the order of execution and executes
                simulation steps.

- libdyn_cpp.[cpp,h]: C++ wrapper for libdyn.c

- irpar.sci: Scilab interface for irpar.c for encoding data structures
             that can be read by irpar.c

- libdyn.sci: Scilab interface for creating and connecting blocks. The
              resulting schematic is stored within irpar.c data structures.

- libdyn_generic_exec.cpp: Source code for the command "ortd", which
                           runs schematics.

REAL-TIME BENCHMARK
===================

A benchmark to evaluate the real-time capabilities is available in

modules/synchronisation/demo/TestRealTime.sce

You could also consider this as an example on how to start multiple 
threads with different priorities and CPU assignments.


        
PORTING TO OTHER TARGETS
========================

It is suggested to start at the file realtime.c in the main directory of the 
framework. There are some comment that give the necessary information.
If you plan to port ORTD, consider to contact the author for support.


DEFINED BLOCK ID REGIONS
========================

  ID range | module
  -------- | ----------------
  11001    | muparser
  14001    | rt_server
  15001    | nested
  15100    | synchronisation
  15200    | scicos_blocks
  15300    | ext_process
  15400    | piplines
  15500    | android
  15800    | EDF
  15900    | Random
  22000    | scilab
  39001    | udp_blocks
  39101    | Visualisation
  39201    | Comedi
  39301    | I2C
  60001    | basic_ldblocks
  69001    | matrix



PART  II  --  BACKGROUND INFORMATION
- - - - - - - - - - - - - - - - - - - -



MOTIVATION FOR ANOTHER REAL-TIME FRAMEWORK
==========================================

Since I'm working in the field of biomedical control engineering, my expectations on 
the reliability & flexibility of the software controlling something that is acting on 
the human body are very high. 

When doing experiments, in clinical environments only limited time is available. Faults 
in the operation are critical up to some high extend and technical inadequacies like 
instable software that requires several attempts to fully run can simply not be accepted.
It shall be possible to re-calibrate and make individual adaptations to the implementation
in the shortest time possible. Therefore, instead of code generation, this framework 
uses an interpreter (the Scilab description is assembled faster than generating and 
compiling C-Code). Additionally, new sub-implementations can be designed and loaded 
on-line while e.g. the data acquisition devices stay up and running.

Also I don't like complicated installation procedures where one has to edit files,
adapt directory paths and install numerous other libraries from their sources by hand.
ORTD compiles and installs itself automatically from its source files on the individual
target system. If there is a library installed that could enable additional functionality 
for ORTD, it is used. If not installed, ORTD doesn't complain.

If you find a bug in the software or things that you just do not find so nice,
it would be very cool if give me a short notice/comment by Email. Even complaints like 
"I have to type so much commands when doing ..." are TOTALLY welcome. Every time the user 
of this framework is subjected to an higher programing effort than theoretically necessary, 
something could be improved! Thus let me know. ORTD is supposed be as smart and relaxed 
as possible.




APPENDIX  --  GOOD TO KNOW
- - - - - - - - - - - - - - - - - - - -

CONTRIBUTIONS
=============

Contributions to this framework are gracefully welcome! Thus if you introduced support
for some kind of hardware e.g. GPIO-interfaces from embedded system or missing blocks
for the basic functionality, do not hesitate.


UPCOMMING FEATURES
==================

- Typically appear spontaneously...
- More structured console output of the interpreter command ortd

BUGS
====

- Transfer function blocks are limited by a order of max 20 for the given tf
- Objects created by libdyn_new_feedback can ONLY be used once.
- If an ld_stream block is not connected with another block
  the initialisation of the parameter is not done (but its actually not needed in this case)
- REMARK: All blocks that synchronise simulations
  If a block without inputs has outputs and they are not connected the block will be optimised output.
  However, if it is supposed to synchronise a simulation this not possible then. So please: Connect
  the outputs otherwise there will be surprising results.
- Check if ld_async_simulation waits for the termination of the nested simulation. Intoduce a sleep(10) 
  and see what happening (FIXED)
- Check what is happening if there is actually no block in a simulation.


CONTACT
=======

Christian Klauer
Control Systems Group
TU-Berlin, Germany
Email: <klauer@control.tu-berlin.de>
Web: <http://www.control.tu-berlin.de/User:Christian_Klauer>


CHANGELOG
=========

11.4.11 (v0.99b)
  - small Bug within ld_const function
  - a new printf block

v0.99c
  - Makefile
  - better dokumentation

v0.99d
  - Nicer Scilab interface

v0.99e
  - Modules support
  - muParser (http://muparser.sourceforge.net/) Block supporting math formula within simulations
    there is an example within examples/muparser.sce
  - HART-module (hart.sf.net) for using ORTD in Scicos simulations
  - remote control interface via TCP to set parameters via a new "ld_parameter" block
    enable it in libdyn_generic_exec by the --master_tcpport <port> option
    an example can be found under examples/remote_control.sce
  - Scicos block for integration of ortd schematics

v0.99f
  - sync and async nested ortd simulations (Module nested, experimental)
  - going on with rt_server
  - Joystick integration via a small Python-Script and the rt_server module

v0.99g (4.11.2011)
  - Improved rt_server module
  - Implemented port size checking during schematic generation within Scilab
  - Fixed bugs in nested and basic_blocks module
  - libdyn_generic_exec: Now runs also without RT-Preemption (no root access needed);
    there is now a static version libdyn_generic_exec_static
  - starting with scilab5/Xcos hart module
  - Added documentation parser
  - Some new blocks for vector manipulation

v0.99h ( not release by now )
  - A new and nice graphical installer
  - Bugs: in vector manipulation blocks, ld_muparser_subst (still experimental) 
  - Added new slides in documentation folder
  - Slightly changed the rt_server protocoll
  - Fixed a potential memory leak, when set-up of a schematic fails
  - Manipulation of the Makefile system (Introduced Target Systems)
  - a new ld_steps block and some other new blocks
  - a new ld_statemachine block, which enables a comfortable way of
    setting up a statemachine by use of nested simulations
  - libdyn.sci: more checks with libdyn_check_obj( )
  - Nested schematics can now be exchanged online! (while the controller is running)
    this is done by loading new [i,r]par - files into the simulation nests
  - New Flag COMPF_FLAG_PREPARERESET, which comes before blocks are reset
  - Scicos Block Wrapper (module scicos_blocks) Experimental!
  - Clock-synchronised threaded simulations
  - run external processes and terminate them by the end of the simulation (module ext_process)
  - Global shared object class
  - Scilab help documentation
  - Again a fixed to online replaceable schematics. Now it runs smooth.
  - A nicer interface for Scilab, which looks very much like the Scicos Generic Block
    or a Matlab S-function. Please note that the state update flag will not be called.
  
By date as there will only be the svn-version

  - 9.12.12: Pipeline-module added coming with a ring-buffer implementation for communication between threads  
  - 28.3.13: Android Target available
  - 10.5.13: Added ld_file_save_machine2 macro, that allows to automatically detect port sizes and types
             It is also a nice example on using state machines
  - 10.5.13: Improvement of the simulation synchronisation infrastructure
  - 14.5.13: Improved the definition of blocks and created a new template for a ORTD-module
             in examples/TemplateModule_V2.
  - 14.5.13: Added write support for EDF+ (European Data Format) - files
  - 28.5.13: Easier implementation of objects that can be shared between blocks
  -  3.6.13: A new module for UDP-Communication
  - 20.7.13: * Fixed a bug in synchronisation module that prevented destroying simulations on
             exit. Also the code was rewritten and looks now nicer. Now use
             ld_ClockSync instead of ld_synctimer which was experimental and is now obsolete.
  -          * Added ld_async_simulation for running simulations in thread. This replaces
             the old scheme where ld_nested2 was used.
  -          * Cleaned up class libdyn_nested2 in libdyn_cpp.cpp
  - 17.8.13: More datatypes and blocks for datatype conversion; UDP send/receive blocks (EXPERIMENTAL).
  - 28.8.13: The individual CPU and the priority to run a thread can be adjusted with ld_async_simulation.
  - 2.9.13:  Blocks that allow to notify (send events) to threads and corresponding blocks for receiving
             these events. Seems to work -- however an example is missing.
  - 14.11.13:Added support for checking the blocks parameters. Some blocks will now give usefull error 
             messages, if wrong parameter types are applied (e.g. if an ortd-signals is applied, while
             a constant is expected.).
  - 15.11.13: Added libdyn_simulation_SyncCallback_terminateThread to force the termination of threads
              in synchronised blocks. By now the parameter signal must be =0. pthread_cancel is called
              on the thread.
  - 6.1.14:   Added: I/O to external Processes (module ext_process)
  - 16.2.14:  Cleaned up code; task priorities fixed.
  - 14.3.14:  Fixed a Bug in the irpar-c++ class
              Re-organisation of the code for nesting schematics
              New blocks for a new communication framework
  - 16.4.14:  Removed the dependency on an rt_server TCP-server
              The command ortd should now be used without the
              parameter --master-tcpport . Also changed this in the
              template for Realtime control.
  -           Added the new communication framework: Packet Framework: Build e.g. web interfaces to your ORTD
              implementation!
  -           Added examples/PacketFramework
  - 28.6.14:  Added I2C module
  - 13.8.14:  Improved automation capabilities: ld_AutoOnlineExch_dev
              module/scilab: The ORTD-Scilab Toolbox is now automatically available in embedded Scilab
              removed some very obsolete warning messages in libdyn.c
              fixed a threading Bug that appeared very seldom.
  - 27.8.14:  Fixed many memory leaks that were detected by valgrind
  -  7.9.13:  More memory leaks were fixed.
              Introduced io.h / io.cpp. Currently used to output the output of the Scilab-module's Scilab instance
  - 24.9.14:  New target MACOSX, Bug fixes, new command ortdrun
  - 4.3.15:   Bug fixes: Shared Objects were not destructed; fixed UDPSendTo to
              send UDP-data when state update is called: Fixes delayed transmission
  - 29.3.15:  Added Random module
  - month 2/3/4 15: Advanced PaPi integration
  - 2.5.16:   Data-Streaming to PaPI has been made more efficient, as multiple streams are scombined to one stream
              that is demultiplexed by PaPI
  - 2.5.16:   Support for 64-Bit ARM (aarch64) e.g. Odroid C2 (introduced in rev 608 but not in 607!)
  - 11.5.17:  ld_ForLoopNest2 allows to build Triigered Subsystems like in Simulink



