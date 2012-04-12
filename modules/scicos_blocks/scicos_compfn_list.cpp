#include "scicos_compfn_list.h"



void scicos_compfn_list_register(char *name, void *compfnptr)
{
  
  std::string name__(name);
  
  std::cout << "Registering scicos block " << name__ << "\n";
  printf("ptr %p\n", compfnptr);
  
  scicos_compfn_list.insert( std::make_pair(name__, compfnptr) );
}

void *scicos_compfn_list_find(char *name)
{
  std::string name__(name);
  
  std::cout << "Searching for scicos block " << name__ << "\n";
  
  scicos_compfn_mapT::iterator it;
  it = scicos_compfn_list.find(name__);
  
  void *compfnptr = it->second;
  printf("found %p\n", compfnptr);
}
