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


directory_entry::directory_entry()
{
  content.allocated_name = true;
}

void directory_entry::set(char* name, void* data)
{
  this->content.name = malloc(  strlen(name) );
  content.allocated_name = true;
  strcpy( content.name , name );
  
  content.data = data;
}

void directory_entry::destruct()
{
  if (content.allocated_name == true)
    free(content.name);
}





directory_leaf::directory_leaf()
{

}


void directory_leaf::destruct()
{

}

bool directory_leaf::add_entry(char* name)
{
//   entries.insert(std::make_pair(id, ));
}
