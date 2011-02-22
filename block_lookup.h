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


//
// A list of computational functions
//

#define LIBDYN_COMPFN_TYPE_LIBDYN 10
#define LIBDYN_COMPFN_TYPE_LIBDYN_STATIC 11
#define LIBDYN_COMPFN_TYPE_SCICOS 20


struct lindyn_comp_func_list_ele_t {
  int blockid;
  int comp_fn_type;  // libdyn block or scicos block
  void *comp_fn;
  
  struct lindyn_comp_func_list_ele_t * next;  
};

struct lindyn_comp_func_list_head_t {
  int num_elements;

  struct lindyn_comp_func_list_ele_t * list_head;  
  struct lindyn_comp_func_list_ele_t * list_tail;  
};


struct lindyn_comp_func_list_head_t * libdyn_new_compfnlist();
int libdyn_compfnlist_add(struct lindyn_comp_func_list_head_t *list, int blockid, int comp_fn_type, void *comp_fn);

// find computational function pointer based on provided block identification number
struct lindyn_comp_func_list_ele_t * libdyn_compfnlist_find_blockid(struct lindyn_comp_func_list_head_t *list, int blockid);
void libdyn_del_compfnlist(struct lindyn_comp_func_list_head_t *list);
