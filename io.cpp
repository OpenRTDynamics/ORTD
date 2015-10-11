/*
    Copyright (C) 2014, 2015  Christian Klauer

    This file is part of OpenRTDynamics, the Real Time Dynamics Framework

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

#include "io.h"



/*


ORTD_IO


*/


ortd_io_internal::ortd_io_internal()
{
//   fprintf(stderr, "New ortd_io_internal interface\n");
}

void ortd_io_internal::PutString(char* s)
{
  fprintf(stderr, "ORTD_IO:%s", s);
//   fprintf(stderr, s);
}

void ortd_io_internal::PutBuffer(char* header, size_t hlen, char* s, size_t len)
{
  fprintf(stderr, "ORTD_BINIO %d:", hlen+len);
  fwrite( (void*) header, hlen, 1, stderr );
  fwrite( (void*) s, len, 1, stderr);
}






 


void ortd_io::PutString(libdyn *sim, char* s)
{
//   sim->get->ortd_io->PutString(s);
// FIXME Segfaults if no master is used
  sim->get_master()->ortd_io->PutString(s);
}

void ortd_io::PutString(libdyn_nested2 *sim, char* s)
{
//   if (sim == NULL){
//     fprintf(stderr, "simnest2\n");
// //     throw 1;
//     return;
//   }
    sim->get_master()->ortd_io->PutString(s);
}

void ortd_io::PutString(dynlib_simulation_t* sim, char* s)
{
//    ortd_io::PutString( (libdyn_nested2 *) sim->SimnestClassPtr , s );
   
   libdyn_master * master = (libdyn_master *) sim->master;
   master->ortd_io->PutString(s);
}

void ortd_io::PutString(dynlib_block_t *block, char *s)
{
  ortd_io::PutString(block->sim, s);
}


void ortd_io::PutBuffer(libdyn* sim, char* header, size_t hlen, char* s, size_t len)
{
 sim->get_master()->ortd_io->PutBuffer(header, hlen, s, len);
}


void ortd_io::PutBuffer(dynlib_simulation_t* sim, char* header, size_t hlen, char* s, size_t len)
{
   libdyn_master * master = (libdyn_master *) sim->master;
   master->ortd_io->PutBuffer(header, hlen, s, len);

}



