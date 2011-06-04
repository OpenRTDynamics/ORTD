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

#include "parameter_manager.h"

int callback_set(rt_server_command *cmd, rt_server *rt_server_src)
{
  printf("my callback was called for command <%s>\n", cmd->command_name);
  
  cmd->send_answer(rt_server_src, "Parameter was set\n");
}

int callback_get(rt_server_command *cmd, rt_server *rt_server_src)
{
  printf("my callback was called for command <%s>\n", cmd->command_name);
  
  cmd->send_answer(rt_server_src, "Parameter is: \n");
}


parameter_manager::parameter_manager(rt_server_threads_manager* rts_thmng)
{
  this->rts_thmng = rts_thmng;
  
  // create root directory
  this->root_directory = new directory_leaf();
  root_directory->set_name("root");
  
  // register commands
  this->rts_thmng->add_command("set_param", 1, &callback_set );
  this->rts_thmng->add_command("get_param", 1, &callback_get );
  
}
