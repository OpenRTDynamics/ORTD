#include <pthread.h>
#include <stdio.h>
#include <map>

class directory_leaf;

class directory_entry {
  public:
    directory_entry();
    void set(char *name, void *data);

    typedef struct {
      char *name; bool allocated_name;
      void *data; // either pointer to data or parameter_directory *dir
      int type;
      int shortcut_id;
    } direntry;

    
    direntry content;
    
    void destruct();
};

class directory_leaf {
  public:
    directory_leaf();
    
    void set_name(const char *name);
    
    double * get_double_value_ptr(char *parname);
    bool add_entry(char *name);


    //  if ret = 1 then  *object IS "direntry *"   , *object is "parameter_directory *dir"
    int access(char * path, void **object);
    
    // collapse all subdirs
    void destruct();
  private:
    directory_leaf * upper_level;
    
    //parameter_manager * pmgr;    
    
    // list of contents
    class directory_entry only_one_entry;
    
    // list of entries
    typedef std::map<int, directory_entry *> entry_map_t;
    entry_map_t entries;
    
    // call access
    directory_entry * get_entry(char *name);
};