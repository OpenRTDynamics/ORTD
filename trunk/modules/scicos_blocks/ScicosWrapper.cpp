

#include "ScicosWrapper.h"
#include <string.h>

// #include "stdio.h"




//#define SCICOS_WRAPPER_INCLUDESCICOSFN 1


#ifdef SCICOS_WRAPPER_INCLUDESCICOSFN
extern "C" { // C-functions that are used by Scicos-Blocks

// /*----------------------------------------  Lapack messag function */
// void C2F(xerbla)(SRNAME,INFO,L)
//      char *SRNAME;
//      int *INFO;
//      long int L;
// {
//   printf("** On entry to %s, parameter number %d"
//          "  had an illegal value\n",SRNAME,*INFO);
// }

void set_block_error(int err)
{
  return;
}

int get_phase_simulation()
{
  return 1;
}

void * scicos_malloc(size_t size)
{
  return malloc(size);
}

void scicos_free(void *p)
{
  free(p);
}

double get_scicos_time()
{
  return 0.0;
}

void do_cold_restart()
{
  return;
}

void sciprint (char *fmt)
{
  return;
}


}
#endif







ScicosWrapper::ScicosWrapper(  )
{

}


ScicosWrapper::~ScicosWrapper()
{

}


void ScicosWrapper::setInPtr(int i, double* p)
{
  cosblock.inptr[i] = p;
}

void ScicosWrapper::setOutPtr(int i, double* p)
{
  cosblock.outptr[i] = p;
}

void ScicosWrapper::setInSize(int i, int size)
{
  cosblock.insz[i] = size;
}

void ScicosWrapper::setOutSize(int i, int size)
{
  cosblock.outsz[i] = size;
}


//void ScicosWrapper::initStructure( int Nrpar, int Nipar, int *ipar, int *rpar, int Nin, int Nout, double **inptr, double **outptr, int *insizes, int *outsizes, int Nz, double *z_init )
void ScicosWrapper::initStructure( int (*compfn)(scicos_block * block, int flag), int Nipar, int Nrpar, int *ipar, double *rpar, int Nin, int Nout, int Nz, double *z_init )
{
  
    this->compfn = compfn;
  
  // parameters
  cosblock.type = 4; //
  cosblock.ztyp = 0;
  cosblock.ng = 0;
  cosblock.nz = 0;
  cosblock.noz = 0;
  cosblock.nevout = 0; // Number of output events REQUIREMENT IS 0
  cosblock.nmode = 0;
  
  // block parameters
  cosblock.nrpar = Nrpar; //
  cosblock.nrpar = Nipar; //
  cosblock.nopar = 0; //
  
  cosblock.ipar = ipar;
  cosblock.rpar = rpar;
  cosblock.oparptr = NULL; //(void**) malloc(10000); // FIXME: Don't what this is good for
  
  
  
  cosblock.nin = Nin; //
  cosblock.nout = Nout; //
  
  
  
  
//   double a,b,c;
//   
//   double *inptr[] = { &a, &b, &c };
//   double *outptr[] = { &a, &b, &c };
//   
//   cosblock.inptr = inptr;
//   cosblock.outptr = outptr;
//   
  
  // in / outptr
  cosblock.inptr = (void**) malloc( sizeof(void*)* Nin );
  cosblock.outptr = (void**) malloc( sizeof(void*)* Nout );
/*  memcpy( cosblock.inptr, inptr, sizeof(void*)* Nin );
  memcpy( cosblock.outptr, outptr, sizeof(void*)* Nout );*/
  
  
  // insizes / outsizes
  this->insizes = (int*) malloc( sizeof(int)*Nin );
  this->outsizes = (int*) malloc( sizeof(int)*Nout );
  
/*  memcpy(this->insizes, insizes, sizeof(int)*Nin );
  memcpy(this->outsizes, outsizes, sizeof(int)*Nout );*/
  
  cosblock.insz = this->insizes;
  cosblock.outsz = this->outsizes;
  
  
  
  
  // states
  cosblock.nz = Nz;  
  cosblock.z = (double*) malloc( sizeof(double)*Nz );
  memcpy(cosblock.z, z_init, sizeof(double)*Nz);
  
  // object states ???
  this->ozptr[0] = malloc(1000);
  this->ozptr[1] = malloc(1000);
  this->ozptr[2] = malloc(1000);
  this->ozptr[3] = malloc(1000);
  this->ozptr[4] = malloc(1000);  
  
//   this->z_states_vector = (double*)  malloc( sizeof(double)*Nz );
//   memcpy(this->z_states_vector, z_init, sizeof(double)*Nz);  
  cosblock.ozptr = (void**) this->ozptr;  
  
  
  // out events
  cosblock.evout = NULL;
  
  // work ptr
  cosblock.work = &workPtr;
  
  
  // unused elements since only time-discrete things are possible
  cosblock.x = NULL;
  cosblock.xd = NULL;
  cosblock.res = NULL;
}

void ScicosWrapper::freeStructure()
{
  free(cosblock.inptr);
  free(cosblock.outptr);
  free(insizes);
  free(outsizes);
  free(cosblock.z);
  
  
}


int ScicosWrapper::Cinit()
{
//   printf("scicoswrap: init %p\n", compfn);
  (*compfn)(&cosblock, 4);
}

int ScicosWrapper::CCalcOutputs()
{
//   printf("scicoswrap: outputs\n");
  
  cosblock.nevprt = 1;
  (*compfn)(&cosblock, 1);
  
  
  double **p = (double**) this->cosblock.outptr;
  
//   printf(" %f \n", p[0][0]);
}

int ScicosWrapper::CStateUpd()
{
  cosblock.nevprt = 1;
//   printf("scicoswrap: supdate\n");
  (*compfn)(&cosblock, 2);
}

int ScicosWrapper::Cdestruct()
{
//   printf("scicoswrap: destruct\n");
  (*compfn)(&cosblock, 5);
}




//   /* set blk struc. of 'cstblk4' (type 4 - blk nb 1) */
//   block_SuperBlock[0].type   = 4;
//   block_SuperBlock[0].ztyp   = 0;
//   block_SuperBlock[0].ng     = 0;
//   block_SuperBlock[0].nz     = 0;
//   block_SuperBlock[0].noz    = 0;
//   block_SuperBlock[0].nrpar  = 1;
//   block_SuperBlock[0].nopar  = 0;
//   block_SuperBlock[0].nipar  = 0;
//   block_SuperBlock[0].nin    = 0;
//   block_SuperBlock[0].nout   = 1;
//   block_SuperBlock[0].nevout = 0;
//   block_SuperBlock[0].nmode  = 0;
//   if ((block_SuperBlock[0].evout  = calloc(block_SuperBlock[0].nevout,sizeof(double)))== NULL) return 0;
//   if ((block_SuperBlock[0].inptr  = malloc(sizeof(double*)*block_SuperBlock[0].nin))== NULL) return 0;
//   if ((block_SuperBlock[0].insz   = malloc(3*sizeof(int)*block_SuperBlock[0].nin))== NULL) return 0;
//   if ((block_SuperBlock[0].outsz  = malloc(3*sizeof(int)*block_SuperBlock[0].nout))== NULL) return 0;
//   if ((block_SuperBlock[0].outptr = malloc(sizeof(double*)*block_SuperBlock[0].nout))== NULL) return 0;
//   block_SuperBlock[0].outptr[0] = SuperBlock_block_outtbptr[0];
//   block_SuperBlock[0].outsz[0]  = 1;
//   block_SuperBlock[0].outsz[1]  = 1;
//   block_SuperBlock[0].outsz[2]  = SCSREAL_N;
//   block_SuperBlock[0].z = &(z[0]);
//   block_SuperBlock[0].rpar=&(RPAR1[0]);
//   block_SuperBlock[0].work = (void **)(((double *)work)+0);


// typedef struct {
//   int nevprt;
//   voidg funpt ;
//   int type;
//   int scsptr;
//   int nz;
//   double *z;
//   int noz;
//   int *ozsz;
//   int *oztyp;
//   void **ozptr;
//   int nx;
//   double *x;
//   double *xd;
//   double *res;
//   int nin;
//   int *insz;
//   void **inptr;
//   int nout;
//   int *outsz;
//   void **outptr;
//   int nevout;
//   double *evout;
//   int nrpar;
//   double *rpar;
//   int nipar;
//   int *ipar;
//   int nopar;
//   int *oparsz;
//   int *opartyp;
//   void **oparptr;
//   int ng;
//   double *g;
//   int ztyp;
//   int *jroot;
//   char *label;
//   void **work;
//   int nmode;
//   int *mode;
// } scicos_block;


