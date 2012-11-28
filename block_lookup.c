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


// block_lookup.h
#include <stdlib.h>
#include <stdio.h>

#include "block_lookup.h"

struct lindyn_comp_func_list_head_t * libdyn_new_compfnlist()
{
  struct lindyn_comp_func_list_head_t * list_header = (struct lindyn_comp_func_list_head_t *) malloc(sizeof(struct lindyn_comp_func_list_head_t));
  
  list_header->list_head = NULL;
  list_header->num_elements = NULL;
  list_header->list_tail = NULL;
  
  return list_header;  // Grr: The lack of this line was a long living Bug! FIXED at 10. July 2011
}

int libdyn_compfnlist_add(struct lindyn_comp_func_list_head_t *list, int blockid, int comp_fn_type, void *comp_fn)
{
  struct lindyn_comp_func_list_ele_t * element = (struct lindyn_comp_func_list_ele_t *) malloc(sizeof(struct lindyn_comp_func_list_ele_t));

  
  element->blockid = blockid;
  element->comp_fn = comp_fn;
  element->comp_fn_type = comp_fn_type;
  
  if (list->list_head == NULL) { // first element in list
    list->list_head = element;
    list->list_tail = element;
    element->next = NULL;
    
  } else { // add element to list head
    element->next = list->list_head;
    list->list_head = element;
  }
  
//   printf("register block %d\n", blockid);

  return 1;
}

// find computational function pointer based on provided block identification number
struct lindyn_comp_func_list_ele_t * libdyn_compfnlist_find_blockid(struct lindyn_comp_func_list_head_t *list, int blockid)
{
  if (list == NULL)
    return NULL;
  
  if (list->list_head == NULL)
    return NULL;
  
  struct lindyn_comp_func_list_ele_t * current = list->list_head;
  
  do {   
    if (current->blockid == blockid) {
      return current; // found!
    }
    
    current = current->next;


  } while (current != NULL);
  
  return NULL; // nothing found
}

// find block id based on the computational function pointer 
struct lindyn_comp_func_list_ele_t * libdyn_compfnlist_find_comp_fn(struct lindyn_comp_func_list_head_t *list, void *comp_fn)
{
  if (list == NULL)
    return NULL;
  
  if (list->list_head == NULL)
    return NULL;
  
  struct lindyn_comp_func_list_ele_t * current = list->list_head;
  
  do {   
    if (current->comp_fn == comp_fn) {
      return current; // found!
    }
    
    current = current->next;


  } while (current != NULL);
  
  return NULL; // nothing found
}

void libdyn_compfnlist_Show_comp_fn(struct lindyn_comp_func_list_head_t *list, void *comp_fn) 
{
  struct lindyn_comp_func_list_ele_t * entr = libdyn_compfnlist_find_comp_fn(list, comp_fn);
  
  if (entr != NULL) {
    fprintf(stderr, "Block with comp_fn ptr %p has the blockid %d\n", entr->comp_fn, entr->blockid); 
  }
  
}

// find computational function pointer based on provided block identification number
struct lindyn_comp_func_list_ele_t * libdyn_compfnlist_List(struct lindyn_comp_func_list_head_t *list)
{
  if (list == NULL)
    return NULL;
  
  if (list->list_head == NULL)
    return NULL;
  
  fprintf(stderr, "                         \n");
  fprintf(stderr, "List of registered blocks\n");
  fprintf(stderr, "                         \n");
  fprintf(stderr, "-------------------------\n");
  fprintf(stderr, "-BlockId--|-Comp. fn ptr-\n");
  fprintf(stderr, "----------|--------------\n");
  
  struct lindyn_comp_func_list_ele_t * current = list->list_head;
  
  do {   
//     if (current->blockid == blockid) {
//       return current; // found!
//     }
    fprintf(stderr, "%d | %p", current->blockid, current->comp_fn);
    
    current = current->next;


  } while (current != NULL);

  fprintf(stderr, "----------|--------------\n");

  return NULL; // nothing found
}

void libdyn_del_compfnlist(struct lindyn_comp_func_list_head_t *list)
{
  if (list->list_head == NULL) {
    free(list);
    return;
  }

  struct lindyn_comp_func_list_ele_t * current = list->list_head;
  struct lindyn_comp_func_list_ele_t * tmp;
  
  do {
    tmp = current->next;
    free(current);
    
    current = tmp;
  } while (current != NULL);
  
  
  return;
}


