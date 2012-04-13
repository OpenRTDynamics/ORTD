

#include <stdio.h>
#include <map>
#include <string>
#include <stdlib.h>
#include <iostream> 


extern "C" void ORTD_scicos_compfn_list_register(char *name, void *compfnptr);
extern "C" void *scicos_compfn_list_find(char *name);

typedef std::map< std::string , void *> scicos_compfn_mapT;





