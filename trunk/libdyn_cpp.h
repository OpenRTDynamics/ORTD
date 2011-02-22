/*
    Copyright (C) 2010, 2011  Christian Klauer

    This file is part of OpenRTDynamics, the Real Time Dynamic Toolbox

    OpenRTDynamics is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenRTDynamics is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with OpenRTDynamics.  If not, see <http://www.gnu.org/licenses/>.
*/


//

extern "C" {
  #include "libdyn.h"  
}

class libdyn {
private:
  struct dynlib_simulation_t *sim;
  struct libdyn_io_config_t iocfg;
  int error;
  
public:
 /**
  * \brief Set-up a new libdyn instance
  * \param Nin number of inputs
  * \param insizes_ an array of size Nin containing port sizes for the input ports
  * \param Nout number of outputs
  * \param outsizes_ an array of size Nout containing port sizes for the output ports
  */
  libdyn(int Nin, int* insizes_, int Nout, int* outsizes_);
  
 /**
  * \brief Delete everything
  */
  void destruct();
  
 /**
  * \brief Configure pointer to input port source variables
  * \param in number of input port
  * \param inptr array of a double variables that will be used as input vector
  */
  bool cfg_inptr(int in, double *inptr);

 /**
  * \brief Set-up a simulation schematic based on integer and real vectors (irpar encoding; see irpar.c)
  * \brief this was encoded by a scilab script
  *
  * \brief irpar_load_from_afile is commonly used for loading these vector from files
  *
  * \param ipar array of integers containing irpar encoding
  * \param rpar array of doubles containing irpar encoding 
  * \param boxid irpar-ID of the schematic container
  *
  * \return if there is an error compiling the schematic a value less than 0 is returned
  */
  int irpar_setup(int *ipar, double *rpar, int boxid);
  
  
 /**
  * \brief Trigger events according to a given bitmask
  * \brief This should be called before "simulation_step".
  * \brief These events are routed to the blocks
  *
  * \param mask each bit stands for an event, which is one if the event should occur
  */
  void event_trigger_mask(int mask);

  /**
  * \brief One step in simulation
  * \brief if update_states == 1, then COMPF_FLAG_UPDATESTATES is send to all computational functions
  * \brief if update_states == 0, then COMPF_FLAG_CALCOUTPUTS is send to all computational functions in the correct sequence
  */
  void simulation_step(int update_states);

 /**
  * \brief Get a simulation output value (only for output ports of size one)
  * \param out Number of output port
  */
  double get_skalar_out(int out);

  /**
  * \brief get a pointer to an array of a simulation output port
  * \param out Number of output port
  */
  double * get_vec_out(int out);

  /**
  * \brief Add a user defined computational function (UNTESTED)
  * \param blockid the block id to be used (this has to be a unique one)
  * \param comp_fn pointer to the compuational function
  */
  bool add_libdyn_block(int blockid, void *comp_fn);
  
  /**
  * \brief Dump a list of all created blocks
  */  
  void dump_all_blocks();
};

