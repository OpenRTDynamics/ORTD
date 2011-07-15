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

/*
 * This is the computational function for the ortd generic Scicos/Xcos - Block
 *
*/

#include "libdyn_cpp.h"


#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
//#include <scicos/scicos_block.h>
#include <scicos_block4.h>
#include "getstr.h"

// #include "irpar.h"

/*

*/


#define ipar_p (&block->ipar[0])
#define rpar_p (&block->rpar[0])

#define use_master (ipar_p[1])
#define master_tcp_port (ipar_p[2])
#define nEvents (ipar_p[3])
#define shematic_id (ipar_p[4])
#define fname_len (ipar_p[10])
#define encfname_ptr (&ipar_p[11])


#define in_p(i) ( &(GetRealInPortPtrs(block, ((i)+1) ))[0] )
#define out_p(i) ( &(GetRealOutPortPtrs(block, ((i)+1) ))[0] )
//#define out_vec ( &(GetRealOutPortPtrs(block,1))[0] )


struct hart_libdynDev {
  libdyn_nested * simnest;
  libdyn_master * master;
  irpar *param;
  
  int Nin; int Nout;
  int *insizes, *outsizes;
  int *intypes, *outtypes;
  
  int error;
};

struct calc_stuff_t {
  double ret;
};

void hart_libdyn_bloc_calc_stuff(scicos_block *block, struct hart_libdynDev * comdev , struct calc_stuff_t *ret, int update_states) //
{
  int i,j;

  if (comdev->error < 0)
    return;
  
  for (i=0; i< comdev->Nin ; ++i) {
    comdev->simnest->copy_inport_vec(i, in_p(i));
  }
  
  // map scicos events to libdyn events
  // convert scicos events to libdyn events (acutally there is no conversion)
  int eventmask = GetNevIn(block);
  comdev->simnest->event_trigger_mask(eventmask);
  
  if (update_states == 1) {
    comdev->simnest->simulation_step(1);   
  } else {
    comdev->simnest->simulation_step(0);
    
    for (i=0; i< comdev->Nout ; ++i) {
      comdev->simnest->copy_outport_vec(i, out_p(i));
    }
    
  }
  
  

}

int hart_libdyn_bloc_init(scicos_block *block,int flag)
{
  struct hart_libdynDev * comdev = (struct hart_libdynDev *) malloc(sizeof(struct hart_libdynDev));
  *block->work = (void *) comdev;

  comdev->error = 0;

  comdev->insizes = NULL;
  comdev->outsizes = NULL;
  comdev->intypes = NULL;
  comdev->outtypes = NULL;
 
  // decode the filename
  #define MAX_FNAMELEN 256
  char fname[MAX_FNAMELEN];
  if (fname_len > MAX_FNAMELEN) {
    printf("filename too long\n");
    comdev->error = -10;
    return -1;
  }
    
  par_getstr( fname, encfname_ptr, 0, fname_len );
  printf("using irpar files: %s\n", fname);
  
  // Configure in- and out port sizes
  int Nipar = GetNipar(block);
  int Nrpar = GetNrpar(block);

  printf("Nin %d, Nout %d\n", GetNin(block), GetNout(block));

  int Nin = GetNin(block);
  int Nout = GetNout(block);

  int *insizes = (int*) malloc(sizeof(int) * Nin);
  int *outsizes = (int*) malloc(sizeof(int) * Nout);
  int *intypes = (int*) malloc(sizeof(int) * Nin);
  int *outtypes = (int*) malloc(sizeof(int) * Nout);

  comdev->Nin = Nin;
  comdev->Nout = Nout;
  
  comdev->insizes = insizes;
  comdev->outsizes = outsizes;
  comdev->intypes = intypes;
  comdev->outtypes = outtypes;
 

  int i;
  for (i=0; i<Nin; ++i) {
    printf("insize[%d]=%d\n", i, GetInPortRows(block,i+1));
    insizes[i] = GetInPortRows(block,i+1);
    intypes[i] = 0; // FIXME
  }
  for (i=0; i<Nout; ++i) {
    printf("outsize[%d]=%d\n", i, GetOutPortRows(block,i+1));
    outsizes[i] = GetOutPortRows(block,i+1);
    outtypes[i] = 0; // FIXME
  }


  bool use_buffered_input = true;  // in- and out port values are buffered
  comdev->simnest = new libdyn_nested(Nin, insizes, intypes, Nout, outsizes, outtypes, use_buffered_input);

  // Load parameters from the files

  printf("Loading irpar files\n");

  comdev->param = new irpar();
  if (comdev->param->load_from_afile(fname) == false)
  {
    comdev->error = -1;
    return -1; 
  }
  
  printf("Loaded irpar files\n");
  
  // If a master should be used install one
  comdev->master = NULL;
  if (use_master == 1) {
    comdev->master = new libdyn_master(RTENV_UNDECIDED, master_tcp_port);
    comdev->simnest->set_master(comdev->master);
  }
  
  printf("loading shematic id %d\n", shematic_id);
  if (comdev->simnest->add_simulation(comdev->param, shematic_id) < 0) {
    comdev->error = -2;
    return -1; 
  }

  


  return 0;
}

int hart_libdyn_bloc_zupdate(scicos_block *block,int flag)
{
  struct hart_libdynDev * comdev = (struct hart_libdynDev *) (*block->work);

  struct calc_stuff_t ret;
  hart_libdyn_bloc_calc_stuff(block, comdev, &ret, 1);
  
  return 0;
}

int hart_libdyn_bloc_outputs(scicos_block *block,int flag)
{
  struct hart_libdynDev * comdev = (struct hart_libdynDev *) (*block->work);

  struct calc_stuff_t ret;
  hart_libdyn_bloc_calc_stuff(block, comdev, &ret, 0);

  
  return 0;
}

#define free_ifnotnull(p)  if ((p) != NULL) free(p)  

int hart_libdyn_bloc_ending(scicos_block *block,int flag)
{
  struct hart_libdynDev * comdev = (struct hart_libdynDev *) (*block->work);
  
  comdev->simnest->destruct();
  delete comdev->simnest;
  
  free_ifnotnull(comdev->insizes);
  free_ifnotnull(comdev->outsizes);
  free_ifnotnull(comdev->intypes);
  free_ifnotnull(comdev->outtypes);
  
  if (comdev->master != NULL) {
    comdev->master->destruct();
    delete comdev->master;
  }
  
  comdev->param->destruct();
  delete comdev->param;
  
  free(comdev);

  return 0;
}


void rt_hart_libdyn(scicos_block *block,int flag)
{
 // printf("called Flag %d\n", flag);

  if (block->nevprt) { //certify that clock port was activated
   // printf("clock port aktivated\n");
  }

  if (flag == 2) { // state update
   //set_block_error(hart_libdyn_bloc_zupdate(block,flag));
   if (block->nevprt) { //certify that clock port was activated
     set_block_error(hart_libdyn_bloc_zupdate(block,flag));  // update states
   }
  }

  if (flag == 4) { /* initialization */
    hart_libdyn_bloc_init(block,flag);
  } else if(flag == 1 || flag == 6) { /* output computation*/
    set_block_error(hart_libdyn_bloc_outputs(block,flag));
  } else  if (flag == 5) { /* ending */
     set_block_error(hart_libdyn_bloc_ending(block,flag));
  }
}

#ifdef __cplusplus
} // end of extern "C" scope
#endif
