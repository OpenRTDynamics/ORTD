/*
  A list of Scicos Blocks and their computational functions is defined here.
*/

#include "stdio.h"
#include "scicos_include/scicos_block4.h"

// This function is provided by the ORTD scicos_blocks module through libortd.[a,so]
extern void ORTD_scicos_compfn_list_register(char *name, void *compfnptr);

// These functions come from the compfn/ subdirectory and are the scicos blocks/schematics FIXME: Autogenerate
extern int  SuperBlock(scicos_block *block, int flag);


// This function is called by rtmain.c
void ORTD_register_scicosblocks() { 
  printf("ORTD_register_scicosblocks was called\n");
  
  // Do the registration for each block FIXME: Autogenerate
  ORTD_scicos_compfn_list_register("SuperBlock", &SuperBlock);
  
  //SuperBlock(NULL, 1);
  
  
  
  
  
}
