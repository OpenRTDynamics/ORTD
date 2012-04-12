#include "stdio.h"
#include "scicos_include/scicos_block4.h"

// This function is provided by the ORTD scicos_blocks module through libortd.[a,so]
extern void ORTD_scicos_compfn_list_register(char *name, void *compfnptr);

// These functions come from the compfn/ subdirectory and are the scicos blocks/schematics
extern int  SuperBlock(scicos_block *block, int flag);

void ORTD_register_scicosblocks() {
  printf("ORTD_register_scicosblocks was called name \n");
  
  ORTD_scicos_compfn_list_register("SuperBlock", &SuperBlock);
}
