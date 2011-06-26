/*
    Copyright (C) 2010, 2011  Christian Klauer

    This file is part of OpenRTDynamics, the Real Time Dynamic Toolbox

    OpenRTDynamics is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenRTDynamics is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with OpenRTDynamics.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "directory.h"


using namespace std;

directory_entry::directory_entry()
{
  content.allocated_name = true;
}

void directory_entry::set(const char* name, void* userptr)
{
  //this->content.name = malloc(  strlen(name) );
  //content.allocated_name = true;
  //strcpy( content.name , name );
  
  content.name = name;
  content.userptr = userptr;
}

void directory_entry::destruct()
{
  if (content.allocated_name == true)
    free((void*) content.name);
}





directory_leaf::directory_leaf()
{
  directory_entry *upper_level = NULL;
  entries.insert( make_pair("..", upper_level ) );
}


void directory_leaf::destruct()
{
  entry_map_t::iterator iter;
  
  for ( iter = entries.begin() ; iter != entries.end(); iter++ ) {
//     string name = iter->first();
//      printf("delete %s\n", name.c_str());
    delete iter->second;
  }

}

void directory_leaf::set_name(const char* name)
{

}


bool directory_leaf::add_entry(char* name, int type, void *belonges_to_class, void* userptr)
{
  // FIXME: also check existance!
  
  //std::string *n = new std::string(name); // FIXME geht das so?
  std::string n(name); // FIXME pot segfault???
  directory_entry *entry = new directory_entry(); 
  
  std::pair <std::string, directory_entry* > paired = std::make_pair(n, entry);
  entries.insert(paired);
  
  entry->set(paired.first.c_str(), userptr); // hopefully this is a const char*
  entry->content.userptr = userptr;
  entry->content.belonges_to_class = belonges_to_class;
  entry->content.type = type;
    
 // entries[name] = entry;
}

directory_entry* directory_leaf::get_entry(char* name)
{
  std::string n(name);
  
  entry_map_t::iterator it;
  it = entries.find(n);
  
  if (it == entries.end()) {
    printf("file not found\n");
    return NULL;
  } 
  
  return it->second;
}

directory_entry * directory_leaf::access1(char* path)
{
  // TODO: iterate into path ...
  
  return get_entry(path);
  
}

directory_entry::direntry* directory_leaf::access2(char* path, void* belonges_to_class)
{
  directory_entry * entr = get_entry(path);
  
  if (entr == NULL)
    return NULL;
  
  if ( entr->content.belonges_to_class != belonges_to_class )
    return NULL;
  
  return &entr->content;
}


directory_entry::direntry * directory_leaf::access2(char* path)
{
  // TODO: iterate into path ...
  
  directory_entry *entr = get_entry(path);

  if (entr == NULL)
    return NULL;
  
  return &entr->content;
}

void directory_leaf::list()
{
//   printf("begin list\n");
  entry_map_t::iterator list_iterator;  
  
   for ( list_iterator = entries.begin() ; list_iterator != entries.end(); list_iterator++ ) {
     directory_entry *entry = list_iterator->second;
     
     
//      printf("  :%p\n", entry);
     if (entry != NULL)
       printf("  %s\n", entry->content.name);
   }
//  printf("end list\n");
}

void directory_leaf::begin_list()
{
//   printf("dumping list\n");
  list_iterator = entries.begin();
}

directory_entry* directory_leaf::get_list_next()
{
  directory_entry *entry2;
  
  do {
    if (list_iterator == entries.end()) {
      // end of list
      return NULL;
    }
    
    entry2 = list_iterator->second;
    
    list_iterator++;
    
    if (entry2 != NULL) {
      // got the next entry
      return entry2;
    }
    
    
  } while ( 1 );
  
  
  

  return NULL;
}


void directory_leaf::end_list()
{

}




/*
 *  Directory Tree class as interface to the outside
 *
 */


int callback_list_dir__(rt_server_command *cmd, rt_server *rt_server_src)
{
  directory_tree *pmgr = (directory_tree *) cmd->userdat;
//   pmgr->callback_list( cmd, rt_server_src );
  pmgr->callback_list_dir(cmd, rt_server_src);
}


// list pwd (current dir)
// and send result to remote client
int directory_tree::callback_list_dir(rt_server_command* cmd, rt_server* rt_server_src)
{
  // got through the list of the pwd directoy
  this->begin_list();
  
  directory_entry::direntry *entr;
  while ( ( entr = this->get_list_next() ) != NULL ) {
    char str[512];
    sprintf(str, "%s %d\n", (char*) entr->name, entr->type ); // FIXME POssible buffer overflow

    cmd->send_answer(rt_server_src, str);
  }
  
  this->end_list();

}



directory_tree::directory_tree() // FIXME remove
{
  root = new directory_leaf();
  
  pwd = root; // start in the root directory
  
  pthread_mutex_init(&this->list_process_mutex, NULL);
}

directory_tree::directory_tree(rt_server_threads_manager* rts)
{
  root = new directory_leaf();
  
  pwd = root; // start in the root directory

//   printf("adding remote cmd ls\n");
  rts->add_command("ls", &callback_list_dir__, this );

  pthread_mutex_init(&this->list_process_mutex, NULL);
}


void directory_tree::destruct()
{
  lock();
  
  root->destruct();
  delete root;
  
  pthread_mutex_destroy(&this->list_process_mutex);
}

directory_entry::direntry* directory_tree::access(char* path, void* belonges_to_class)
{
  lock();
  directory_entry::direntry *dentr = root->access2(path, belonges_to_class);
  unlock();
  
  return dentr;
}

bool directory_tree::add_entry(char* name, int type, void* belonges_to_class, void* userptr)
{
  lock();
  root->add_entry( name, type, belonges_to_class, userptr );
  unlock();
}


// BAUstelle
void directory_tree::begin_list()
{
  
/*  
  pwd->list();
  
  printf("****\n");
   
  // FIXME: lock mutex
   for ( list_iterator = pwd->entries.begin() ; list_iterator != pwd->entries.end(); list_iterator++ ) {
     directory_entry *entry = list_iterator->second;
     
     
     
     printf(":%p\n", entry);
     
//      printf("%s\n", entry->content.name);
   }


list_iterator = pwd->entries.begin();
  
*/
  lock();

  pwd->begin_list();

 
}

directory_entry::direntry* directory_tree::get_list_next()
{
  directory_entry *dentry = pwd->get_list_next();

  if (dentry != NULL) {  
    return &dentry->content;
  } else {    
    return NULL;
  }

  
}


void directory_tree::end_list()
{
// unlock
  unlock();
}


void directory_tree::lock()
{
   pthread_mutex_lock(&this->list_process_mutex);
}

void directory_tree::unlock()
{
   pthread_mutex_unlock(&this->list_process_mutex);
}

