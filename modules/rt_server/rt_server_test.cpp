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

#include "rt_server.h"
#include "malloc.h"
#include <stdio.h>

int callback(rt_server_command *cmd, rt_server *rt_server_src)
{
  printf("my callback was called for command <%s>\n", cmd->command_name);
  
  cmd->send_answer(rt_server_src, "Hallo du\n");
}

int main()
{
  
  
  rt_server_threads_manager * rts_mgr = new rt_server_threads_manager();
  rts_mgr->init_tcp(10000);
  
  rts_mgr->add_command("cmd1", 1, &callback);
  rts_mgr->add_command("cmd2", 2, &callback);
  rts_mgr->add_command("cmd3", 3, &callback);

  rts_mgr->loop();
  
  
  
/*  tcp_server *iohelper =  new tcp_server(10000);
  iohelper->tcp_server_init();
  
  for (;;) {
  tcp_connection *tcpc = iohelper->wait_for_connection();
    
  rt_server *rt_server_i = new rt_server();
  rt_server_i->set_tcp(tcpc);
  
  rt_server_i->add_command("cmd1", 1, &callback);
  rt_server_i->add_command("cmd2", 2, &callback);
  rt_server_i->add_command("cmd3", 3, &callback);
  
  rt_server_i->init(); // the class will start its receiver thread
  }*/
  
  
/*  char line[] = "cmd2 123 654 2.53\n";
  rt_server_i->parse_line(line);
  
  for (;;) sleep(1);*/
  
//   rt_server_i->destruct();
}


