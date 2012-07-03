#include "directory.h"
#include <libdyn_cpp.h>


class nested_onlineexchange {
  public:
    nested_onlineexchange(char *identName, libdyn_nested2 *simnest );
    ~nested_onlineexchange();
    
    int replace_simulation(irpar *irdata, int id, int slot);
    
    int replace_second_simulation(irpar *irdata, int id);
    
  private:    
    std::string identName;
    
    libdyn_nested2 *simnest;
    
    libdyn_master* ldmaster; // extracted from simnest
    
    irpar *current_irdata, *new_irdata;
  
};
