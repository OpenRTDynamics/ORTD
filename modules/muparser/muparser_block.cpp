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

#include <malloc.h>

extern "C" {
#include "libdyn.h"
#include "libdyn_scicos_macros.h"

}


#include "muParser.h"
using namespace mu;




extern "C" {
    int compu_func_muparser(int flag, struct dynlib_block_t *block);
    int libdyn_module_muparser_siminit(struct dynlib_simulation_t *sim, int bid_ofs);
}

class compu_func_muparser_class {
public:
    compu_func_muparser_class(struct dynlib_block_t *block);
    int init();
    void io(int update_states);
    void destruct();

private:
    char *str; // the expression as string
    Parser expr;
    dynlib_block_t* block;


    int Nin; // N inports
    int Nout; // N outports
    int nparam; // number of float parameters for muparser

    // coded expression
    int exprlen;
    int *codedexpr;

    char *paramter_names;
    char *variable_names;


};

compu_func_muparser_class::compu_func_muparser_class(dynlib_block_t* block)
{
    this->block = block;
}

int compu_func_muparser_class::init()
{
    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    Nin = ipar[0];
    Nout = ipar[1];
    nparam = ipar[2];

    exprlen = ipar[3];
    codedexpr = &ipar[4];



    str = (char *) malloc(exprlen+1);

    // Decode filename
    int i;
    for (i = 0; i < exprlen; ++i)
        str[i] = codedexpr[i];

    str[i] = 0; // String termination


    printf("muparser: forula >>%s<<\n", str);

    // register variable / constant names
    const char *names_in[] = {"u1", "u2", "u3", "u4", "u5", "u6", "u7", "u8", "u9", "u10"};
    const char *names_par[] = {"c1", "c2", "c3", "c4", "c5", "c6", "c7", "c8", "c9", "c10"};

    if (Nin > sizeof(names_in)) {
      printf("Error in muparser block irparid=%d\n", block->irpar_config_id);
      printf("muparser: number of block inputs greater than %d\n", sizeof(names_in));
      return -1;
    }
      
    if (nparam > sizeof(names_par)) {
      printf("Error in muparser block irparid=%d\n", block->irpar_config_id);
      printf("muparser: number of constants greater than %d\n", sizeof(names_par));
      return -1;
    }
    

    try
    {
   
        for (i = 0; i < Nin; ++i) {
            double *in = (double *) libdyn_get_input_ptr(block,i);
//             printf("inptr = %p\n", in);

            expr.DefineVar(names_in[i], in);
//             printf(names_in[i]);
        }

        for (i = 0; i < nparam; ++i) {
            expr.DefineConst(names_par[i], rpar[i]);
        }

         expr.SetExpr(str);
    }
    catch (Parser::exception_type &e)
    {
      printf("Error in muparser block irparid=%d\n", block->irpar_config_id);
      printf("expr=\"%s\"\n", str);
        std::cout << e.GetMsg() << std::endl;
        return -1;
    }

    return 0;
}


void compu_func_muparser_class::io(int update_states)
{
    if (update_states==0) {
        double *output = (double*) libdyn_get_output_ptr(block, 0);

//         double *in1 = (double *) libdyn_get_input_ptr(block,0);
//         double *in2 = (double *) libdyn_get_input_ptr(block,1);
//         printf("in = %f %f\n", *in1, *in2);

        double val; // = expr.Eval();


        try
        {
            val = expr.Eval();
        }
        catch (Parser::exception_type &e)
        {
	  printf("Error in muparser block irparid=%d\n", block->irpar_config_id);
 	  printf("expr=\"%s\"\n", str);
            std::cout << e.GetMsg() << std::endl;
        }

        *output  = val;
//         printf("output=%f\n", val);
    }
}

void compu_func_muparser_class::destruct()
{
    free(str);
}


int compu_func_muparser(int flag, struct dynlib_block_t *block)
{

//      printf("comp_func muparser: flag==%d\n", flag);

    double *in;
    int vlen = 1;

    double *rpar = libdyn_get_rpar_ptr(block);
    int *ipar = libdyn_get_ipar_ptr(block);

    int Nin = ipar[0];
    int Nout = ipar[1];


    switch (flag) {
    case COMPF_FLAG_CALCOUTPUTS:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_muparser_class *worker = (compu_func_muparser_class *) libdyn_get_work_ptr(block);

        worker->io(0);

    }
    return 0;
    break;
    case COMPF_FLAG_UPDATESTATES:
    {
        in = (double *) libdyn_get_input_ptr(block,0);
        compu_func_muparser_class *worker = (compu_func_muparser_class *) libdyn_get_work_ptr(block);

        worker->io(1);

    }
    return 0;
    break;
    case COMPF_FLAG_CONFIGURE:  // configure
    {
        int i;
        libdyn_config_block(block, BLOCKTYPE_STATIC, Nout, Nin, (void *) 0, 0);

        for (i = 0; i < Nin; ++i)
            libdyn_config_block_input(block, i, vlen, DATATYPE_FLOAT);

        for (i = 0; i < Nin; ++i)
            libdyn_config_block_output(block, i, vlen, DATATYPE_FLOAT, 1);


    }
    return 0;
    break;
    case COMPF_FLAG_INIT:  // init
    {
        compu_func_muparser_class *worker = new compu_func_muparser_class(block);
        libdyn_set_work_ptr(block, (void*) worker);

        int ret = worker->init();
        if (ret < 0)
            return -1;
    }
    return 0;
    break;
    case COMPF_FLAG_DESTUCTOR: // destroy instance
    {
        compu_func_muparser_class *worker = (compu_func_muparser_class *) libdyn_get_work_ptr(block);

        worker->destruct();

    }
    return 0;
    break;
    case COMPF_FLAG_PRINTINFO:
        printf("I'm a muparser block\n");
        return 0;
        break;

    }
}

//#include "block_lookup.h"

int libdyn_module_muparser_siminit(struct dynlib_simulation_t *sim, int bid_ofs)
{

    // Register my blocks to the given simulation

    int blockid = 11001;
    libdyn_compfnlist_add(sim->private_comp_func_list, blockid, LIBDYN_COMPFN_TYPE_LIBDYN, (void*) &compu_func_muparser);

    printf("libdyn module muparser initialised\n");

}


//} // extern "C"
