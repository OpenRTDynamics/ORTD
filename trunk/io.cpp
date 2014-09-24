#include <stdio.h>

#include "io.h"

ortd_io_internal::ortd_io_internal()
{
//   fprintf(stderr, "New ortd_io_internal interface\n");
}

void ortd_io_internal::PutString(char* s)
{
  fprintf(stderr, "ORTD_IO:");
  fprintf(stderr, s);
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

// void ortd_io::printf(dynlib_simulation_t* sim, char* format, ...)
// {
// //    ortd_io::PutString( (libdyn_nested2 *) sim->SimnestClassPtr , s );
//    
// //    printf(format, __VA_ARGS__);
//    
//    libdyn_master * master = (libdyn_master *) sim->master;
//    master->ortd_io->PutString(s);
// }
// 


