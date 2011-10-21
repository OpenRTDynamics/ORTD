

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
//#include <scicos/scicos_block.h>
#include <scicos_block4.h>
#include "getstr.h"

#include "log.h"

/*

*/
#define block_version (block->ipar[0])

#define blk_version (block->ipar[0])
#define veclen (block->ipar[1])

#define len_fname (block->ipar[4])
#define fname_coded (&block->ipar[5])


#define in_vec ( &(GetRealInPortPtrs(block,1))[0] )
//#define out_vec ( &(GetRealOutPortPtrs(block,1))[0] )


struct filewriterDev {
  char fname[256];
  struct filewriter_t *fw;
};

struct calc_stuff_t {
  double ret;
};

void filewriter_bloc_calc_stuff(scicos_block *block, struct filewriterDev * comdev , struct calc_stuff_t *ret, int update_states) //
{
  if (update_states == 1) {
    log_dfilewriter_log(comdev->fw, in_vec);
  }
}

int filewriter_bloc_init(scicos_block *block,int flag)
{
  struct filewriterDev * comdev = (struct filewriterDev *) malloc(sizeof(struct filewriterDev));

  par_getstr(comdev->fname, fname_coded, 0, len_fname);
  printf("filewriter scicosblock: %s\n", comdev->fname);
  
  comdev->fw = log_dfilewriter_new(veclen, 100, comdev->fname);
  
//  comdev->dyn1_a = block->rpar[10];

  *block->work = (void *) comdev;

  return 0;
}

int filewriter_bloc_zupdate(scicos_block *block,int flag)
{
  struct filewriterDev * comdev = (struct filewriterDev *) (*block->work);

  struct calc_stuff_t ret;
  filewriter_bloc_calc_stuff(block, comdev, &ret, 1);
  
  return 0;
}

int filewriter_bloc_outputs(scicos_block *block,int flag)
{
  struct filewriterDev * comdev = (struct filewriterDev *) (*block->work);

  struct calc_stuff_t ret;
  filewriter_bloc_calc_stuff(block, comdev, &ret, 0);

  
/*  out_vec[0] = 1;
  out_vec[1] = 1.1;
  out_vec[2] = 1.2;*/
  
  return 0;
}

int filewriter_bloc_ending(scicos_block *block,int flag)
{
  struct filewriterDev * comdev = (struct filewriterDev *) (*block->work);
  
  log_dfilewriter_del(comdev->fw);
  
  free(comdev);

  return 0;
}


void rt_filewriter(scicos_block *block,int flag)
{
 // printf("called Flag %d\n", flag);

  if (block->nevprt) { //certify that clock port was activated
   // printf("clock port aktivated\n");
  }

  if (flag == 2) { // state update
   //set_block_error(filewriter_bloc_zupdate(block,flag));
   if (block->nevprt) { //certify that clock port was activated
     filewriter_bloc_zupdate(block,flag);  // update states
   }
  }

  if (flag == 4) { /* initialization */
    filewriter_bloc_init(block,flag);
  } else if(flag == 1 || flag == 6) { /* output computation*/
    filewriter_bloc_outputs(block,flag);
  } else  if (flag == 5) { /* ending */
     filewriter_bloc_ending(block,flag);
  }
}

#ifdef __cplusplus
} // end of extern "C" scope
#endif
