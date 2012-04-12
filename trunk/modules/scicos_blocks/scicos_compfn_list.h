

#include <stdio.h>
#include <map>
#include <string>
#include <stdlib.h>
#include <iostream> 


extern "C" void scicos_compfn_list_register(char *name, void *compfnptr);

typedef std::map< std::string , void *> scicos_compfn_mapT;

scicos_compfn_mapT scicos_compfn_list;




