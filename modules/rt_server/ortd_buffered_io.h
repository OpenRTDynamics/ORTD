#include <stdio.h>
#include <sstream>

#ifndef _ORTD_BUFFERED_IO_H
#define _ORTD_BUFFERED_IO_H 1


class ortd_buffered_io {
public:
  ortd_buffered_io(int fd);
  ~ortd_buffered_io();
  
  int waitforaline();
  int writeln(char *line);
  
  std::stringstream sstream_in;

private:
  int fd;
  
  
};

#endif