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


bool directory_leaf::add_entry(char* name, void *belonges_to_class, void* userptr)
{
  //std::string *n = new std::string(name); // FIXME geht das so?
  std::string n(name); // FIXME pot segfault???
  directory_entry *entry = new directory_entry(); 
  
  std::pair <std::string, directory_entry* > paired = std::make_pair(n, entry);
  entries.insert(paired);
  
  entry->set(paired.first.c_str(), userptr); // hopefully this is a const char*
  entry->content.userptr = userptr;
  entry->content.belonges_to_class = belonges_to_class;
    
  entries[name] = entry;
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









// int callback_list_dir__(rt_server_command *cmd, rt_server *rt_server_src)
// {
//   parameter_manager *pmgr = (parameter_manager *) cmd->userdat;
// //   pmgr->callback_list( cmd, rt_server_src );
// }




directory_tree::directory_tree()
{
  root = new directory_leaf();
  
  pwd = root; // start in the root directory
}

void directory_tree::destruct()
{
  root->destruct();
  delete root;
}

directory_entry::direntry* directory_tree::access(char* path, void* belonges_to_class)
{
  return root->access2(path, belonges_to_class);
}

bool directory_tree::add_entry(char* name, void* belonges_to_class, void* userptr)
{
  root->add_entry( name, belonges_to_class, userptr );
}


// BAUstelle
void directory_tree::begin_list()
{
  // FIXME: lock mutex
  list_iterator = pwd->entries.begin();
  
//   for ( list_iterator = pwd->entries.begin() ; list_iterator != pwd->entries.end(); list_iterator++ ) {
//     directory_entry *entry = list_iterator->second;
//   }

}

directory_entry::direntry* directory_tree::get_list_next()
{
  if (list_iterator != pwd->entries.end()) {
    directory_entry *entry = list_iterator->second;
    
    list_iterator++;
    
    printf("next list entry: %s\n", entry->content.name);
    return &entry->content;
  }

  return NULL;
}


void directory_tree::end_list()
{
// unlock
}

