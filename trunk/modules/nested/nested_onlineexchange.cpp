// #ifdef REMOTE

#include "nested_onlineexchange.h"
#include <libdyn_cpp.h>

nested_onlineexchange::nested_onlineexchange(char* identName, libdyn_nested* simnest)
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
    
  dtree->add_entry(identName, ORTD_DIRECTORY_ENTRYTYPE_NESTEDONLINEEXCHANGE, this, this);
  
}

int nested_onlineexchange::replace_simulation(irpar* irdata, int id, int slot)
{
  // remove the old simulation
  simnest->del_simulation( slot );
  
  // delete the old now unused irpar data for the old simulation
  if (this->current_irdata != NULL)
    delete this->current_irdata;
  
  // install the new one
  this->current_irdata = irdata;
  simnest->add_simulation(slot, current_irdata->ipar, current_irdata->rpar, id);
}


nested_onlineexchange::~nested_onlineexchange()
{
  directory_tree *dtree = ldmaster->dtree;
  dtree->delete_entry(identName);
  
  delete this->current_irdata;
}


// #endif