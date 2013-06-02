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

#ifndef _IRPAR_H
#define _IRPAR_H 1



/**
  * \brief irpar encoding:
  * \brief
  * \brief Some functions for combining various data structures
  * \brief into one rpar + ipar set
  * \brief encoding is done by irpar.sci
  */


//#define IRPAR_HEADER_ELEMENT_LEN 4
#define IRPAR_HEADER_ELEMENT_LEN 6
#define IRPAR_RVEC 1
#define IRPAR_RMAT 2
#define IRPAR_TF 3
#define IRPAR_IVEC 4

#define IRPAR_LIBDYN_BLOCK 100
#define IRPAR_LIBDYN_CONNLIST 101

struct irpar_header_element_t {
  int id;
  int typ;
  int *ipar_ptr;
  double *rpar_ptr;
  int rpar_len, ipar_len;
  
  int setindex; // a unique index to this param set
};

struct irpar_rvec_t { // Vektor
  double *v;
  int n; // len
};

struct irpar_ivec_t { // Vektor
  int *v;
  int n; // len
};

struct irpar_rmat_t { // mat
  double *v;
  int n, m; // size
};

struct irpar_tf_t { // tf
  double *num;
  double *den;
  int degnum, degden; // size
};

//#define irpar_get_ipar(ipar, rpar, i) ( ipar[0]*IRPAR_HEADER_ELEMENT_LEN )


/**
  * \brief Convert int[] to char[] (decode ascii characters)
  * 		Allocates memedor for ret_str, which you shoud free()
  */
void irpar_getstr(char ** ret_str, int par[], int init, int len);


int irpar_get_nele(int *ipar, double *rpar);

/**
  * \brief Get a combination of an integer and double vector from irpar encoding (usefull for nesting or generic structures)
  *
  * \param ret pointer to a structure that will be filled by this function. 
  * \param ipar pointer to ipar
  * \param rpar pointer to rpar_len
  * \param id the id of the container (this was specified while encoding with scilab)
  *
  * \return 0 in case everything was ok; else -1 is returned
  *
  */

int irpar_get_element_by_id(struct irpar_header_element_t *ret, int *ipar, double *rpar, int id);

/**
  * \brief Get a real vector from irpar encoding
  *
  * \param ret pointer to a structure that will be filled by this function. 
  *            ret->v will be set to the vector data, 
  *            while ret->n is the length of this vector.
  * \param ipar pointer to ipar
  * \param rpar pointer to rpar_len
  * \param id the id of the vector (this was specified while encoding with scilab)
  *
  * \return 0 in case everything was ok; else -1 is returned
  *
  */

int irpar_get_rvec(struct irpar_rvec_t *ret, int *ipar, double *rpar, int id);

/**
  * \brief Get a integer vector from irpar encoding
  *
  * \param ret pointer to a structure that will be filled by this function. 
  *            ret->v will be set to the vector data, 
  *            while ret->n is the length of this vector.
  * \param ipar pointer to ipar
  * \param rpar pointer to rpar_len
  * \param id the id of the vector (this was specified while encoding with scilab)
  *
  * \return 0 in case everything was ok; else -1 is returned
  *
  */

int irpar_get_ivec(struct irpar_ivec_t *ret, int *ipar, double *rpar, int id);

/**
  * \brief Get a transfer function from irpar encoding
  *
  * \param ret pointer to a structure that will be filled by this function. 
  * \param ipar pointer to ipar
  * \param rpar pointer to rpar_len
  * \param id the id of the tf (this was specified while encoding with scilab)
  *
  * \return 0 in case everything was ok; else -1 is returned
  *
  */

int irpar_get_tf(struct irpar_tf_t *ret, int *ipar, double *rpar, int id);

/**
  * \brief Get a matrix from irpar encoding. Matrix is unwrapped to a vector in scilab by A(:), whereby A is the Matrix.
  *
  * \param ret pointer to a structure that will be filled by this function. Contains dimensions n,m afterwards.
  * \param ipar pointer to ipar
  * \param rpar pointer to rpar_len
  * \param id the id of the matrix (this was specified while encoding with scilab)
  *
  * \return 0 in case everything was ok; else -1 is returned
  *
  */

int irpar_get_rmat(struct irpar_rmat_t *ret, int *ipar, double *rpar, int id);

/**
  * \brief Load a irpar set from a file
  * \brief memory for both vectors is reserved, while the pointers to this memory
  * \brief are returned written to *ipar and *rpar
  * \brief Additionally the vector lengths are written to *ilen and *rlen
  *
  * \param fname_i filename of integer file
  * \param fname_r filename of double file
  *
  * \return 0 in case everything was ok; else -1 is returned
  *
  */
int irpar_load_from_afile(int **ipar, double **rpar, int *ilen, int *rlen, char *fname_i, char *fname_r);


// Macro fro loading a double scalar err has to defined as int before calling this
#define irpar_get_rscalar(ipar, rpar, id, out) { \
  struct irpar_rvec_t tmp354236_; \
  err = irpar_get_rvec(&tmp354236_, (ipar), (rpar), (id)); \
  if (err == -1) { \
    printf("irpar id %d not found!\n", (id)); \
  } \
  *(out) = tmp354236_.v[0]; \
}


#define irpar_get_iscalar(ipar, rpar, id, out) { \
  struct irpar_ivec_t tmp354236_; \
  err = irpar_get_ivec(&tmp354236_, (ipar), (rpar), (id)); \
  if (err == -1) { \
    printf("irpar id %d not found!\n", (id)); \
  } \
  *(out) = tmp354236_.v[0]; \
}


#define irpar_get_rvec_forcelen(ipar, rpar, id, outp, len) { \
  struct irpar_rvec_t tmp354236_; \
  err = irpar_get_rvec(&tmp354236_, (ipar), (rpar), (id)); \
  if (err == -1 || tmp354236_.n != (len)) { \
    printf("irpar id %d not found or length != %d!\n", (id), (len)); \
    err == -1; \
  } \
  *(outp) = tmp354236_.v; \
}

#define irpar_get_ivec_forcelen(ipar, rpar, id, outp, len) { \
  struct irpar_ivec_t tmp354236_; \
  err = irpar_get_ivec(&tmp354236_, (ipar), (rpar), (id)); \
  if (err == -1 || tmp354236_.n != (len)) { \
    printf("irpar id %d not found or length != %d!\n", (id), (len)); \
    err == -1; \
  } \
  *(outp) = tmp354236_.v; \
}



#endif