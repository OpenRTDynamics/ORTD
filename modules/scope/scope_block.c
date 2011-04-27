/*
COPYRIGHT (C) 2006  Roberto Bucher (roberto.bucher@supsi.ch)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/

//extern "C" {

//#include <machine.h>
//#include <scicos_block4.h>
#include "libdyn.h"
#include "libdyn_scicos_macros.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(RTAI)
// RTAI

#include <rtai_netrpc.h>
#include <rtai_msg.h>
#include <rtai_mbx.h>
#include "rtmain.h"

#define MBX_RTAI_SCOPE_SIZE           5000

extern char *TargetMbxID;

#else
// NORMAL LINUX CONTEXT

  #include "rt_preempt_scope.h"

#endif




static void init(scicos_block *block, char *name, int nch)
// static void init(struct dynlib_block_t *block)
{ 
  char tmp[20];
//   char name[7];
//  int nch = GetNin(block);
  int i;
  
  int lskd = block->ipar;

/*  int * ipar = GetIparPtrs(block);  
  int nch=GetInPortRows(block,1);*/
  int nt = nch+1;
//   if (ipar[0]>20)                                                                                                                               
//         ipar[0]=20;                                                                                                                             
//   par_getstr(tmp,ipar,1,ipar[0]);                                                                                                         


  char * scopeName;
  scopeName=(char*)malloc(strlen(name)+1);
  strcpy(scopeName,name);

#if defined(RTAI)
  
  char ** traceName;
     traceName =  (char**)malloc(sizeof(char*)*nch);

  for (i=0;i<nch;i++){
	traceName[i]=(char*)malloc(strlen(tmp)+1);
	sprintf(tmp,"Trace %d",i);
	strcpy(traceName[i], tmp);
  }



  MBX *mbx;

  rtRegisterScope(scopeName,traceName , nch);

  get_a_name(TargetMbxID,name);

  mbx = (MBX *) RT_typed_named_mbx_init(0,0,name,(MBX_RTAI_SCOPE_SIZE/(nt*sizeof(double)))*(nt*sizeof(double)),FIFO_Q);

  if(mbx == NULL) {
    fprintf(stderr, "Cannot init mailbox\n");
    exit_on_error();
  }

  (block->work) = (void *) mbx;
#else

  printf("New RT_PREEMPT SCOPE\n");

  struct rt_preempt_scope_t * scope = rt_preempt_scope_new();
  (block->work) = (void *) scope;
  rt_preempt_scope_setup(scope, SCOPE_TYPE_SCOPE, nch + 1, scopeName);
  
  free(scopeName);
#endif
}

static void inout(scicos_block *block)
{
  double *u;

//  int ntraces=GetNin(block);
  int ntraces=GetInPortRows(block,1);
  struct {
    double t;
    double u[ntraces];
  } data;
  int i;

  //data.t=get_scicos_time();
  data.t = block->sim->stepcounter;


  u = libdyn_get_input_ptr(block,0); //  u = block->inptr[0];
  for (i = 0; i < ntraces; i++) {
     data.u[i] =  u[i];
  }
  
  #if defined(RTAI)
  MBX * mbx = (block->work);
  RT_mbx_send_if(0, 0, mbx, &data, sizeof(data));
  #else
  
  struct rt_preempt_scope_t * scope = (block->work);
  rt_preempt_scope_send_double(scope, &data);
  
  #endif
}

static void end(scicos_block *block)
{
  char scopeName[20];
   int * ipar = GetIparPtrs(block);
   
   #if defined(RTAI)
   MBX * mbx = (block->work);
   RT_named_mbx_delete(0, 0, mbx);
   #else

   struct rt_preempt_scope_t * scope = (block->work);
   rt_preempt_scope_unregister(scope);
   rt_preempt_scope_destruct();
   
   #endif
   
//    par_getstr(scopeName,ipar,1,ipar[0]);
//    printf("Scope %s closed\n",scopeName);

}

// void rtscope(scicos_block *block,int flag)
// {
//   if (flag==1){          
//     inout(block);
//   }
//   else if (flag==5){     /* termination */ 
//     end(block);
//   }
//   else if (flag ==4){    /* initialisation */
//     init(block);
//   }
// }



int compu_func_scope(int flag, struct dynlib_block_t *block)
{
  
  //printf("comp_func gain: flag==%d\n", flag);
  int Nout = 0;
  int Nin = 1;

  double *in;

  double *rpar = libdyn_get_rpar_ptr(block);
  int *ipar = libdyn_get_ipar_ptr(block);

  int vlen = ipar[0];
  int namelen = ipar[1];
  int *codedname = &ipar[2];
  
  
  switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {  
    }
      return 0;
      break;
    case COMPF_FLAG_UPDATESTATES:
    {
      in = (double *) libdyn_get_input_ptr(block,0);
//       char *str = (char *) block->work;

      inout(block);
    
    } 
      return 0;
      break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
      
      libdyn_config_block(block, BLOCKTYPE_DYNAMIC, Nout, Nin, (void *) 0, 0); 
      libdyn_config_block_input(block, 0, vlen, DATATYPE_FLOAT); 
    } 
      return 0;
      break;
    case COMPF_FLAG_INIT:  // init
    {
//      char filename[250];
      char *str = (char *) malloc(namelen+1);
      
      // Decode filename
      int i;
      for (i = 0; i < namelen; ++i)
	str[i] = codedname[i];
      
      str[i] = 0; // String termination
      
      
      printf("New scope %s\n", str);
      
      init(block, str, vlen);
      
      free(str);
      
    }
      return 0;
      break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
//       char *str = (char *) block->work;
      end(block);

    }
      return 0;
      break;      
    case COMPF_FLAG_PRINTINFO:
      printf("I'm a scope block\n");
      return 0;
      break;
      
  }
}




//} // extern "C"