


// Defines some macros as defined in scicos_block4.h for usage with libdyn blocks


typedef struct dynlib_block_t scicos_block;
//#define scicos_block struct dynlib_block_t

#define GetIparPtrs(ldblock) ((ldblock)->ipar)
#define GetRparPtrs(ldblock) ((ldblock)->rpar)


#define GetInPortRows(ldblock, i) (libdyn_get_inportsize(ldblock, i))
#define GetOutPortRows(ldblock, i) (libdyn_get_outportsize(ldblock, i))

#define get_scicos_time() (0) // FIXME



// #define libdyn_get_output_ptr(block, out) ( ((block)->Nout <= out) ? 0 : (block)->outlist[(out)].data  )
// #define libdyn_get_input_ptr(block, in) (  ((block)->Nin <= in) ? 0 : (block)->inlist[(in)].data )
// #define libdyn_get_ipar_ptr(block) ((block)->ipar)
// #define libdyn_get_rpar_ptr(block) ((block)->rpar)
// #define libdyn_get_opar_ptr(block) ((block)->opar)
// #define libdyn_get_work_ptr(block) ((block)->work)
// #define libdyn_set_work_ptr(block, work_) ((block)->work = (work_))
// #define libdyn_get_Ninports(block) ((block)->Nin)
// #define libdyn_get_Noutports(block) ((block)->Nout)
// #define libdyn_get_inportsize(block, i) ((block)->inlist[(i)].len)
// #define libdyn_get_outportsize(block, i) ((block)->outlist[(i)].len)


