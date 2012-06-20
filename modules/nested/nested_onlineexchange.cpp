// #ifdef REMOTE

#include "nested_onlineexchange.h"
#include <libdyn_cpp.h>

nested_onlineexchange::nested_onlineexchange(const char* identName, libdyn_nested2* simnest)
{
  this->identName = identName;
  this->simnest = simnest;
  this->current_irdata = NULL; // HISTORY: Here was a very stupid bug: it was == insted of = . Fixed on 20.6.2012
  
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
  
//   fprintf(stderr, "nested_onlineexchange created. this=%p current_irdata=%p\n", this, current_irdata);
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
  fprintf(stderr, "nested_onlineexchange: removing old simulation\n");
//   fprintf(stderr, "nested_onlineexchange created. this=%p current_irdata=%p\n", this, current_irdata);
  
  int standby_slot = 0;
  int slot = 1; // exchange the second slot
  
  // remove the old simulation
  int ret = simnest->del_simulation( slot, standby_slot );
  if (ret < 0)
    return -1;
  
  
  
  // delete the old now unused irpar data for the old simulation
  if (this->current_irdata != NULL) {
    fprintf(stderr, "delete irpar of old simulation\n");
    delete this->current_irdata;    
  }
  
  // install the new one
  fprintf(stderr, "load new irpar for the new simulation\n");
    
  this->current_irdata = irdata;
  ret = simnest->add_simulation(slot, current_irdata->ipar, current_irdata->rpar, id);
  if (ret < 0) {
    fprintf(stderr, "Error while setting up the new simulation\n");
    // error setting up the simulation
    // irdata is unused now
    delete this->current_irdata;
    this->current_irdata = NULL;
    
    fprintf(stderr, "irpar unloaded again\n");
  }
    
  return ret;
}


nested_onlineexchange::~nested_onlineexchange()
{
  fprintf(stderr, "Destroying nested_onlineexchange for %s\n", identName);
  
  directory_tree *dtree = ldmaster->dtree;
  dtree->delete_entry((char*) identName);
  
  if (this->current_irdata != NULL) {
    fprintf(stderr, "Running delete this->current_irdata  ptr=%p\n", this->current_irdata); 
    delete this->current_irdata; 
  }
  
  fprintf(stderr, "Sucessfully destroyed nested_onlineexchange for %s\n", identName);
}


// #endif