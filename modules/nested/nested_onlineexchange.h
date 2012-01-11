#include "directory.h"
#include <libdyn_cpp.h>


class nested_onlineexchange {
  public:
    nested_onlineexchange(const char *identName, libdyn_nested *simnest );
    ~nested_onlineexchange();
    
    int replace_simulation(irpar *irdata, int id, int slot);
    
    int replace_second_simulation(irpar *irdata, int id);
    
  private:
    const char *identName;
    libdyn_nested *simnest;
    
    libdyn_master* ldmaster; // extracted from simnest
    
    irpar *current_irdata, *new_irdata;
  
};
