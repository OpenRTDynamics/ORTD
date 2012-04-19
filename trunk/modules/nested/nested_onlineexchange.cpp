// #ifdef REMOTE

#include "nested_onlineexchange.h"
#include <libdyn_cpp.h>

nested_onlineexchange::nested_onlineexchange(const char* identName, libdyn_nested* simnest)
{
  this->identName = identName;
  this->simnest = simnest;
  this->current_irdata == NULL;
  
  this->ldmaster = simnest->ld_master;
  if (this->ldmaster == NULL) {
    fprintf(stderr, "stderr: nested_onlineexchange: needs a libdyn master\n");
    // FIXME: throuw exception
    
    return;
  }
  
  directory_tree *dtree = ldmaster->dtree;
  if (dtree == NULL) {
    fprintf(stderr, "stderr: nested_onlineexchange: needs a root directory_tree\n");
    // FIXME: throuw exception
     
  }
    
  dtree->add_entry((char*) identName, ORTD_DIRECTORY_ENTRYTYPE_NESTEDONLINEEXCHANGE, this, this);
  
}

int nested_onlineexchange::replace_simulation(irpar* irdata, int id, int slot)
{  
  fprintf(stderr, "stderr: nested_onlineexchange: removing old simulation\n");
    
  // remove the old simulation
  int ret = simnest->del_simulation( slot );
  if (ret < 0)
    return -1;
  
  // delete the old now unused irpar data for the old simulation
  if (this->current_irdata != NULL)
    delete this->current_irdata;
  
  // install the new one
  this->current_irdata = irdata;
  ret = simnest->add_simulation(slot, current_irdata->ipar, current_irdata->rpar, id);
  
  
  return ret;
}

int nested_onlineexchange::replace_second_simulation(irpar* irdata, int id)
{
  fprintf(stderr, "stderr: nested_onlineexchange: removing old simulation\n");
  
  int standby_slot = 0;
  int slot = 1; // exchange the second slot
  
  // remove the old simulation
  int ret = simnest->del_simulation( slot, standby_slot );
  if (ret < 0)
    return -1;
  
  // delete the old now unused irpar data for the old simulation
  if (this->current_irdata != NULL)
    delete this->current_irdata;
  
  // install the new one
  this->current_irdata = irdata;
  ret = simnest->add_simulation(slot, current_irdata->ipar, current_irdata->rpar, id);
  if (ret < 0) {
    // error setting up the simulation
    // irdata is unused now
    delete irdata;
  }
    
  return ret;
}


nested_onlineexchange::~nested_onlineexchange()
{
  fprintf(stderr, "Destroying nested_onlineexchange for %s\n", identName);
  
  directory_tree *dtree = ldmaster->dtree;
  dtree->delete_entry((char*) identName);
  
  if (this->current_irdata != NULL)
    delete this->current_irdata;  
  
  fprintf(stderr, "Sucessfully destroyed nested_onlineexchange for %s\n", identName);
}


// #endif