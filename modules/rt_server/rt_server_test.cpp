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

// #include "rt_server.h"
// #include "directory.h"
#include "parameter_manager.h"

#include "malloc.h"
#include <stdio.h>

int callback(rt_server_command *cmd, rt_server *rt_server_src)
{
  printf("my callback was called for command <%s>\n", cmd->command_name);
  
  cmd->send_answer(rt_server_src, "Hallo du\n");
}

int main()
{
  directory_tree * dtree = new directory_tree();
  
/*  directory_leaf *root = dtree->root;
  
  root->add_entry("test1", NULL, (void*) 0x123);
  root->add_entry("test2", NULL, (void*) 0x321);
  
  directory_entry::direntry *inode = root->access2("test1");
  printf("%x\n", inode->userptr);*/

  dtree->add_entry("test1", NULL, (void*) 0x123);
  dtree->add_entry("test2", NULL, (void*) 0x456);
  
  directory_entry::direntry *inode = dtree->access( "test1", NULL );
  printf("%x\n", inode->userptr);
  
//   root->destruct();
  
  
  rt_server_threads_manager * rts_mgr = new rt_server_threads_manager();
  rts_mgr->init_tcp(10000);
  
  rts_mgr->add_command("cmd1", &callback, NULL);
  rts_mgr->add_command("cmd2", &callback, NULL);
  rts_mgr->add_command("cmd3", &callback, NULL);


  
  parameter_manager * pmgr = new parameter_manager( rts_mgr, dtree );

  parameter *p1 = pmgr->new_parameter("parameter1", DATATYPE_FLOAT, 10 );
  
  rts_mgr->start_main_loop_thread();

  
  
  pmgr->destruct();
  rts_mgr->destruct();
  
  for (;;) sleep(1);
  
  
  
  
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


