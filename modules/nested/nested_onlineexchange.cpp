// #ifdef REMOTE

#include "nested_onlineexchange.h"
#include <libdyn_cpp.h>



//  TODO re-implement using a globally shared object

nested_onlineexchange::nested_onlineexchange(char* identName, libdyn_nested2* simnest)
{
  this->identName = identName;
   
  
  this->simnest = simnest;
  this->current_irdata = NULL; // HISTORY: Here was a very stupid bug: it was == insted of = . Fixed on 20.6.2012
  
  this->ldmaster = simnest->ld_master;
  if (this->ldmaster == NULL) {
    fprintf(stderr, "nested_onlineexchange: nested_onlineexchange: needs a libdyn master\n");
    // FIXME: throuw exception
    
    return;
  }
   
  directory_tree *dtree = ldmaster->dtree;
  if (dtree == NULL) {
    fprintf(stderr, "nested_onlineexchange: nested_onlineexchange: needs a root directory_tree\n");
    // FIXME: throuw exception
     
  }
    
  dtree->add_entry((char*) this->identName.c_str(), ORTD_DIRECTORY_ENTRYTYPE_NESTEDONLINEEXCHANGE, this, this);
  
//   // TODO: add entries to a  list stored in the simnest2 class
//   simnest->addElement((char*) this->identName.c_str(), ORTD_DIRECTORY_ENTRYTYPE_NESTEDONLINEEXCHANGE, this );
   
  
//   fprintf(stderr, "nested_onlineexchange created. this=%p current_irdata=%p\n", this, current_irdata);
}

int nested_onlineexchange::replace_simulation(irpar* irdata, int id, int slot)
{  
  fprintf(stderr, "nested_onlineexchange: destructing the currently active simulation in slot %d\n", slot);
    
  // remove the old simulation
  int ret = simnest->del_simulation( slot );
  if (ret < 0)
    return -1;
  
  // delete the old now unused irpar data for the old simulation
  if (this->current_irdata != NULL)
    delete this->current_irdata;
  
  fprintf(stderr, "nested_onlineexchange: loading the new simulation into slot %d\n", slot);
  
  // install the new one
  this->current_irdata = irdata;
  ret = simnest->add_simulation(slot, current_irdata->ipar, current_irdata->rpar, id);
  
  
  return ret;
}

int nested_onlineexchange::replace_second_simulation(irpar* irdata, int id)
{
  
  int standby_slot = 0;
  int slot = 1; // exchange the second slot

  fprintf(stderr, "nested_onlineexchange: destructing the currently active simulation in slot %d\n", slot);
  
  // remove the old simulation
  int ret = simnest->del_simulation( slot, standby_slot );
  if (ret < 0)
    return -1;
  
  
  
  // delete the old, now unused irpar data for the old simulation
  if (this->current_irdata != NULL) {
#ifdef DEBUG
    fprintf(stderr, "nested_onlineexchange: delete irpar of old simulation\n");
#endif
    delete this->current_irdata;    
  }
  
  // install the new one
#ifdef DEBUG
  fprintf(stderr, "load new irpar for the new simulation\n");
#endif

  
  fprintf(stderr, "nested_onlineexchange: loading the new simulation into slot %d\n", slot);
  
  this->current_irdata = irdata;
  ret = simnest->add_simulation(slot, current_irdata->ipar, current_irdata->rpar, id);
  if (ret < 0) {
#ifdef DEBUG
     fprintf(stderr, "nested_onlineexchange: Error while setting up the new simulation\n");
#endif
    // error setting up the simulation
    // irdata is unused now
    delete this->current_irdata;
    this->current_irdata = NULL;
  }
    
  return ret;
}


nested_onlineexchange::~nested_onlineexchange()
{
#ifdef DEBUG
   fprintf(stderr, "Destroying nested_onlineexchange for %s\n", identName.c_str());
#endif
  
  directory_tree *dtree = ldmaster->dtree;
  dtree->delete_entry((char*) identName.c_str());
  
  // TODO enable this instead
//   simnest->deleteElement( (char*) identName.c_str() );
  
  if (this->current_irdata != NULL) {
#ifdef DEBUG
    fprintf(stderr, "nested_onlineexchange: Running delete this->current_irdata  ptr=%p\n", this->current_irdata); 
#endif
    delete this->current_irdata; 
    this->current_irdata = NULL;
    
#ifdef DEBUG
     fprintf(stderr, "nested_onlineexchange: Sucessfully destroyed nested_onlineexchange for %s\n", identName.c_str());
#endif
  } else {
    fprintf(stderr, "nested_onlineexchange: Could not find %s\n", identName.c_str());
  }
    
  
  
}


// #endif