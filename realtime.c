#include <time.h>

#ifdef _TTY_POSIX_
long int ortd_mu_time()
{

  struct timeval mytime;
  struct timezone myzone;

  gettimeofday(&mytime, &myzone);
  return (1000000*mytime.tv_sec+mytime.tv_usec);

} /* mu_time */
#endif
