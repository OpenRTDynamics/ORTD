extern "C" {
#include "scicos_block4.h"
};


class ScicosWrapper {
public:
  ScicosWrapper();
  ~ScicosWrapper();

  void initStructure( int (*compfn)(scicos_block * block, int flag), int Nrpar, int Nipar, int* ipar, double* rpar, int Nin, int Nout, int Nz, double* z_init );
  void setOutPtr(int i, double *p);
  void setInPtr(int i, double *p);
  void setInSize(int i, int size);
  void setOutSize(int i, int size);
  
  int Cinit();
  int CStateUpd();
  int CCalcOutputs();
  int Cdestruct(); 
  
  void freeStructure();
  
private:
  
  scicos_block cosblock;
  
  int (*compfn)(scicos_block * block, int flag);
  
  int *cosipar;
  double cosrpar;

  int Nin, Nout;
  int *insizes, *outsizes;
  
  void *workPtr;
  
};


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



