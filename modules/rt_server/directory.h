#ifndef _DIRECTORY_H
#define _DIRECTORY_H 1



#include <pthread.h>
#include <stdio.h>
#include <map>
#include <string>

#include <pthread.h>

#include "rt_server.h"



#define ORTD_DIRECTORY_ENTRYTYPE_PARAMETER 1
#define ORTD_DIRECTORY_ENTRYTYPE_STREAM 2
#define ORTD_DIRECTORY_ENTRYTYPE_IPARDATA 3
#define ORTD_DIRECTORY_ENTRYTYPE_RPARDATA 4
#define ORTD_DIRECTORY_ENTRYTYPE_IRPARSET 5
#define ORTD_DIRECTORY_ENTRYTYPE_NESTEDONLINEEXCHANGE 6

class directory_leaf;

class directory_entry {
  public:
    directory_entry();
    void set(const char* name, void* userptr);

    typedef struct {
      const char *name; bool allocated_name;
      void *userptr; // either pointer to data or parameter_directory *dir
      void *belonges_to_class; // the class instance that will be called
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
    
    double * get_double_value_ptr(char *parname); // FIXME: REMOVE?
    bool add_entry(char *name, int type, void *belonges_to_class, void* userptr);


    //  if ret = 1 then  *object IS "direntry *"   , *object is "parameter_directory *dir"
    
    directory_entry::direntry *access2(char* path);
    
    // same as above but checks wheter the file is add_entry'ed with belonges_to_class.
    // if not returns NULL
    directory_entry::direntry *access2(char* path, void *belonges_to_class);
    directory_entry * access1(char* path);
    bool delete_file(char *path);

     // list of entries
    typedef std::map<std::string, directory_entry *> entry_map_t;
    entry_map_t entries;

    
    // list entries
    void list(); // printf list
    
    void begin_list();
    directory_entry *get_list_next();
    void end_list();
    
    entry_map_t::iterator list_iterator; // not public
    
    
    // collapse all subdirs
    void destruct();
  private:
    directory_leaf * upper_level;
    
    //parameter_manager * pmgr;    
    
    // list of contents
    class directory_entry only_one_entry;
    
    
    // call access
    directory_entry * get_entry(char *name);
    
    //
    directory_entry* get_and_delete_entry(char* name);
};




// This class is the *only* user interface!

class directory_tree {
  public:
    directory_tree();
    directory_tree( rt_server_threads_manager * rts );
    void destruct();
    
    // create a new file return true if successful
    bool add_entry(char *name, int type, void *belonges_to_class, void* userptr);

    // remove a file
    bool delete_entry(char *name);
    
    directory_entry::direntry* access(char* path, void* belonges_to_class);
    
    // list the pwd directory
    void begin_list();
    directory_entry::direntry* get_list_next();
    void end_list();
    
    // the rt_server callback
    int callback_list_dir(rt_server_command *cmd, rt_server *rt_server_src);
    
    
    void lock();
    void unlock();
private:
    // list
    directory_leaf::entry_map_t::iterator list_iterator;
    pthread_mutex_t list_process_mutex;
    
    
    directory_leaf * root;
    directory_leaf * pwd; // current directory
    
    int magic;
};


#endif