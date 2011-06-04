#include <pthread.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <scicos/scicos_block.h>

#include "libilc.h"
/*

*/

#define debug_mode 1
#define memfak 3

#define u0_ofs 0

#define MTOT 1+3
 // (block->ipar[7])

//extrakt(comdev->u_it, i, comdev->outcounter)
//
//       comdev->u_it[i*comdev->N_samples + comdev->N_samples-1];
//  <=>  extract(comdev->u_it, i, comdev->N_samples-1, comdev->N_samples);

# define extract(arrptr, ch, n, Nsam) arrptr[ch*Nsam + n]


//#define extract_double_arr_ptr(arrptr, ch, N_samples) (arrptr + sizeof(double)*N_samples*ch)
#define extract_double_arr_ptr(arrptr, ch, Nsam) (&(arrptr[ch*Nsam + 0]))


//
// Hole den "sample" ten Werte des "ch" ten Kanals der Referenz
//
#define get_reference_input_vec(ch,sample) block->inptr[(ch)+1][sample]

#define get_reference_input_vecptr(ch) (block->inptr[(ch)+1])

// Greift auf Kopie zu, die vor Ausführung des ilc Updates erneuert wird
#define get_reference_input_copy_vecptr(ch) (&(comdev->inptr_ref_cpy[(ch)*comdev->N_samples]))


// Vektorielle Ausgänge des Blocks
#define set_output(ch,sample,out) block->outptr[ch+2][sample] = out
#define get_output_vecptr(ch) (&(block->outptr[ch+2][0]))

#define get_inp_l (&(block->rpar[block->ipar[0]])) 
#define get_inp_q (&(block->rpar[block->ipar[0] + block->ipar[1]])) 

// Greift auf Kopie zu
#define get_inp_l_copy (&(comdev->rpar_cpy[comdev->ipar_cpy[0]])) 
#define get_inp_q_copy (&(comdev->rpar_cpy[comdev->ipar_cpy[0] + comdev->ipar_cpy[1]])) 


struct ilcDev {
  int N_ipar, N_rpar;
  int N_samples, N_impulse, num_channel, hold_type, filtfiltmode;
  double *u_it, *u_it_cp;
  double *sample_data;
  double *old_sample_data;
  double *tmp_data, *tmp_data2, *tmp_ldata;
  int out_counter;
  int first_run;
  int active;
  int mtot;

  pthread_t thread_ilc_update;
  pthread_mutex_t thread_mutex;
  pthread_mutex_t ilcdata_mutex;
  pthread_cond_t thread_condition;
  int thread_command;

  scicos_block *block;

  double *inptr_ref_cpy, *rpar_cpy;
  int *ipar_cpy;
};

void *rt_ilc_thread_ilc_update(void *data)
{
  int command = 0;

  if (debug_mode==1) printf("ILC: thread; comdevptr = 0x%x\n", (int) data);
//  printf("Scicos block ptr (ilc_update_init) = 0x%x\n", (int) block);

  //scicos_block *block = (scicos_block *) data;
  struct ilcDev * comdev = (struct ilcDev *) data; 
  //scicos_block *block = comdev->block;

  //printf("ipar[5] = %i\n", block->ipar[5]);

  if (debug_mode==1) printf("ILC. thread: Nsamples = %i\n", comdev->N_samples);

  for (;;) {
    pthread_mutex_lock(&comdev->thread_mutex);
    //printf("ILC; thread; mutex locked\n");
    while (comdev->thread_command == 0) pthread_cond_wait(&comdev->thread_condition, &comdev->thread_mutex);
    command = comdev->thread_command;
    printf("ILC; thread; sigal received; command = %i\n", command);
    comdev->thread_command = 0;

    if (command == 1) {
      printf("ILC: Thread: updating ilc; try to pass lock\n");
      pthread_mutex_unlock(&comdev->thread_mutex);
      printf("ILC: Thread: updating ilc; lock passed\n");

      //usleep(1000);
      //sleep(3);

      ilc_bloc_ilc_update(comdev); // Daten sollten bereits geblockt sein
      pthread_mutex_unlock(&comdev->ilcdata_mutex);


    } else if (command == 2) {
      pthread_mutex_unlock(&comdev->thread_mutex);
      printf("ILC: Thread: ending\n");
      pthread_exit(NULL);
      printf("ILC: Thread: ending (This should not appear!)\n");
    }
  }

}


void rt_ilc(scicos_block *block,int flag)
{
  /* 
  int block->nevprt;
  int block->nz;
  double* block->z;
  int block->nx;
  double* block->x;
  double* block->xd;
  double* block->res;
  int block->nin;
  int *block->insz;
  double **block->inptr;
  int block->nout;
  int *block->outsz;
  double **block->outptr;
  int block->nevout;
  int block->nrpar;
  double *block->rpar;
  int block->nipar;
  int *block->ipar;
  int block->ng;
  double *block->g;
  int *block->jroot;
  char block->label[41];
  */



  if (debug_mode==1) printf("called Flag %d\n", flag);

  if (block->nevprt) { //certify that clock port was activated
    if (debug_mode==1) printf("clock port %i aktivated\n", block->nevprt);
  }

  if (flag == 2) { // state update
   //set_block_error(ilc_bloc_zupdate(block,flag));
   if (block->nevprt) { //certify that clock port was activated
     set_block_error(ilc_bloc_zupdate(block,flag));  // update states
   }
  }

  if (flag == 4) {  // initialization 
   ilc_bloc_init(block,flag);
 
  } else if(flag == 1 || flag == 6) { /// output computation
     set_block_error(ilc_bloc_outputs(block,flag));
  } else  if (flag == 5) { // ending 
     set_block_error(ilc_bloc_ending(block,flag));
  }


  set_block_error(0);
}

int ilc_bloc_init(scicos_block *block,int flag)
{
  struct ilcDev * comdev = (struct ilcDev *) malloc(sizeof(struct ilcDev));

  /*
model.ipar=[length(u0);
	    length(L(:));
	    length(Q(:));
	    Nch;
	    hold_type;
	    N_samples;
	    filtfilt
	  ];
model.rpar=[u0(:);
	    L(:);
	    Q(:)
	  ];

  */ 

  // Elemente in ipar und rpar
  comdev->N_ipar = 7;
  comdev->N_rpar = block->ipar[0] + block->ipar[1] + block->ipar[2];

  if (debug_mode==1) printf("Scicos block ptr (init) = 0x%x\n", (int) block);

  comdev->num_channel = block->ipar[3];
  comdev->hold_type = block->ipar[4];
  comdev->filtfiltmode = block->ipar[6];
  comdev->N_samples = block->ipar[0] / comdev->num_channel; // length(u0) / Nch ???
  comdev->N_impulse = comdev->N_samples*2 +1; // Länge der nichtkausalen Impulsantwort der Filter
  comdev->mtot = MTOT;

  if ((comdev->filtfiltmode == 0 | comdev->filtfiltmode == 1) && block->ipar[2] != comdev->N_impulse * comdev->num_channel*comdev->num_channel) {
    if (debug_mode==1) printf("Error in ilr.c: length(Q) != implen * Nch^2; %i != %i \n", block->ipar[1], comdev->N_impulse * comdev->num_channel*comdev->num_channel);
    return 1;   
  }
  if (comdev->filtfiltmode == 2 && block->ipar[2] != comdev->N_samples*comdev->N_samples * comdev->num_channel*comdev->num_channel) {
    if (debug_mode==1) printf("Error in ilr.c: length(Q) != N_samples^2 * Nch^2; %i != %i \n", block->ipar[2], comdev->N_samples*comdev->N_samples * comdev->num_channel*comdev->num_channel);
    return 1;   
  }
  if (comdev->filtfiltmode == 2 && block->ipar[1] != comdev->N_samples*comdev->N_samples * comdev->num_channel*comdev->num_channel) {
    if (debug_mode==1) printf("Error in ilr.c: length(L) != N_samples^2 * Nch^2; %i != %i \n", block->ipar[1], comdev->N_samples*comdev->N_samples * comdev->num_channel*comdev->num_channel);
    return 1;   
  }

/*
  if (block->ipar[1] != comdev->N_impulse * comdev->num_channel*comdev->num_channel) {
    if (debug_mode==1) printf("Error in ilr.c: length(L) != implen * Nch^2)\n");
    return 1;   
  }
*/
  if (block->ipar[0] % comdev->num_channel != 0) {
    if (debug_mode==1) printf("Error in ilr.c: u0 mod problem (length(u0) undividiable num_channel)\n");
    return 1;
  }
  if (block->ipar[5] != comdev->N_samples) {
    if (debug_mode==1) printf("Error in ilr.c: length(u0)/Nch =! N_samples; %i != %i\n", block->ipar[5], comdev->N_samples);
    return 1;
  }   

  comdev->u_it = (double *) malloc(memfak*sizeof(double) * comdev->N_samples * comdev->num_channel);
  comdev->u_it_cp = (double *) malloc(memfak*sizeof(double) * comdev->N_samples * comdev->num_channel);

  comdev->sample_data = (double *) malloc(memfak*sizeof(double) * comdev->N_samples * comdev->num_channel);
  comdev->old_sample_data = (double *) malloc(memfak*sizeof(double) * comdev->N_samples * comdev->num_channel);
  comdev->tmp_data = (double *) malloc(memfak*sizeof(double) * comdev->N_samples * comdev->num_channel);
  comdev->tmp_data2 = (double *) malloc(memfak*sizeof(double) * comdev->N_samples * comdev->num_channel);
  comdev->tmp_ldata = (double *) malloc(memfak*sizeof(double) * comdev->N_samples * comdev->num_channel);

  // Speicher für Kopien von ipar, rpar und den Eingungsvektoren
  comdev->rpar_cpy = (double *) malloc(sizeof(double) * comdev->N_rpar);
  comdev->ipar_cpy = (double *) malloc(sizeof(int) * comdev->N_ipar);
  comdev->inptr_ref_cpy = (double *) malloc(sizeof(double) * comdev->N_samples * comdev->num_channel);

  // Kopiere Daten ipar und rpar
  int i;

  for (i = 0; i < comdev->N_ipar; ++i)
    comdev->ipar_cpy[i] = block->ipar[i];
  for (i = 0; i < comdev->N_rpar; ++i)
    comdev->rpar_cpy[i] = block->rpar[i];


  if (debug_mode==1) printf("Reserved mem for %i samples in %i channel[s]\n", comdev->N_samples * comdev->num_channel, comdev->num_channel);
  if (debug_mode==1) printf("Hold type = %i, N_impulse = %i, mtot= %i\n", comdev->hold_type, comdev->N_impulse, comdev->mtot);

  //comdev->u_it[0] = 1.4;
  //comdev->u_it[1] = 0.7;
 
  for (i = 0; i < (comdev->N_samples*comdev->num_channel); ++i) {
    comdev->u_it[i] = block->rpar[u0_ofs + i];
  }

  comdev->out_counter = 0;
  comdev->first_run = 1;
  comdev->active = 0;

  *block->work = (void *) comdev;

  // Start ilc update thread
  pthread_mutex_init(&comdev->thread_mutex, NULL);
  pthread_mutex_init(&comdev->ilcdata_mutex, NULL);
  pthread_cond_init(&comdev->thread_condition, NULL);
  comdev->thread_command = 0;

  int rc = pthread_create(&comdev->thread_ilc_update, NULL, rt_ilc_thread_ilc_update, (void *) comdev);
  if (rc){
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    return 1;
  }

  comdev->block = block;


  //rt_ilc_send_command(comdev, 2);


  return 0;
}

void rt_ilc_send_command(struct ilcDev *comdev, int command)
{
  pthread_mutex_lock(&comdev->thread_mutex);
  comdev->thread_command = command;
  pthread_mutex_unlock(&comdev->thread_mutex);
  pthread_cond_signal(&comdev->thread_condition);
}


void ilc_bloc_ilc_update(struct ilcDev * comdev)
{
  printf("ilc_up entry; Nsamples=%i\n", comdev->N_samples);

 // struct ilcDev *comdev = (struct ilcDev *) (*block->work);
  int i,j;
 // scicos_block *block = comdev->block;

  printf("Test1 Nsamples=%i\n", comdev->N_samples);
  //printf("Scicos block ptr (ilc_update) = 0x%x\n", (int) block);

  //printf("ipar[5] = %i\n", block->ipar[5]);

  if (debug_mode==1) printf("ILC: u_it neu schreiben\n");

  for (i=0; i<comdev->num_channel; ++i) {
	  double *u_ch = extract_double_arr_ptr(comdev->u_it, i, comdev->N_samples);
	  double *u_ch_cp = extract_double_arr_ptr(comdev->u_it_cp, i, comdev->N_samples);
	  double *tmp_ldata_ch = extract_double_arr_ptr(comdev->tmp_ldata, i, comdev->N_samples);

	  copy_vec(u_ch, u_ch_cp, comdev->N_samples);
	  put_zero_vec(u_ch, comdev->N_samples);
	  put_zero_vec(tmp_ldata_ch, comdev->N_samples);
  }

  for (i=0; i<comdev->num_channel; ++i) // alle Matrizeneinträge
  for (j=0; j<comdev->num_channel; ++j) {
	  // TF von u_i -> y_j
	  //
	  // [u_1   =  [ q_1  q_3     * [ e_1
	  //  u_2]       q_2  q_4  ]      e_2 ]
	  //

	  double *y_ch = extract_double_arr_ptr(comdev->sample_data, i, comdev->N_samples);
	  double *u_ch_cp = extract_double_arr_ptr(comdev->u_it_cp, i, comdev->N_samples);
	  double *r_ch = get_reference_input_copy_vecptr(i);  // THRMOD

	  double *tmp_ldata_ch = extract_double_arr_ptr(comdev->tmp_ldata, j, comdev->N_samples);

	  int index_q = j + i*comdev->num_channel;  // Matrix wie in scilab A(:) durchgehen
	  double *q = extract_double_arr_ptr(get_inp_q_copy, index_q, comdev->N_impulse); // THRMOD
	  double *l = extract_double_arr_ptr(get_inp_l_copy, index_q, comdev->N_impulse); // THRMOD


	  if (debug_mode==1) printf("erster Wert q_%i[0] = %f\n", index_q, q[0]);
	  if (debug_mode==1) printf("erster Wert l_%i[0] = %f\n", index_q, l[0]);

	  //if (debug_mode==1) printf("erster Wert von Q(0) = %f, Q(1) = %f\n", get_inp_q[0], get_inp_q[1]);

	  // e (tmp_data) = y-r
//			put_zero_vec(comdev->tmp_data2, comdev->N_samples);
		  //copy_vec(u_ch_cp, tmp_ldata_ch, comdev->N_samples);

	  // u_n-1 - L*(y-r) = u_n-1 + L*(r-y)
	  linear_combination_vec(-1,1,y_ch,r_ch,comdev->tmp_data, comdev->N_samples);
	  funky_filter(comdev->tmp_data, tmp_ldata_ch, l, comdev->N_samples); // Add L*(y-r) to tmp_data2 (u)
	  if (i==j)
		  add_to_vec(u_ch_cp, tmp_ldata_ch, comdev->N_samples);   // add old u

	  //linear_combination_vec(1,1,u_ch_cp,r_ch,comdev->tmp_data, comdev->N_samples);
	  //linear_combination_vec(1,1,u_ch,comdev->tmp_data2,comdev->tmp_data, comdev->N_samples);
	  //

	  
  }

  for (i=0; i<comdev->num_channel; ++i) // alle Matrizeneinträge 2. Filterung (Q-Filter)
  for (j=0; j<comdev->num_channel; ++j) {
	  // TF von u_i -> y_j
	  //
	  // [u_1   =  [ q_1  q_3     * [ e_1
	  //  u_2]       q_2  q_4  ]      e_2 ]
	  //
	  double *tmp_ldata_ch = extract_double_arr_ptr(comdev->tmp_ldata, i, comdev->N_samples);

	  double *u_ch = extract_double_arr_ptr(comdev->u_it, j, comdev->N_samples);

	  int index_q = j + i*comdev->num_channel;  // Matrix wie in scilab A(:) durchgehen
	  double *q = extract_double_arr_ptr(get_inp_q_copy, index_q, comdev->N_impulse); // THRMOD
	  double *l = extract_double_arr_ptr(get_inp_l_copy, index_q, comdev->N_impulse); // THRMOD


	  if (comdev->filtfiltmode == 0) {	// FIXME: Überprüfen
	    funky_filter(comdev->tmp_data2, u_ch , q, comdev->N_samples); // Apply u = Q*(..)
	  } else if (comdev->filtfiltmode == 1) {  // filtfilt
	    if (debug_mode==1) printf("Vor Q Filter (filtfilt) ");
	    dump_vec(tmp_ldata_ch, comdev->N_samples);

	    // Vorwärts
	    put_zero_vec(comdev->tmp_data, comdev->N_samples);
	    funky_filter(tmp_ldata_ch, comdev->tmp_data , q, comdev->N_samples);

	    // Rückwärts
	    flip_vec(comdev->tmp_data, comdev->N_samples);
	    put_zero_vec(comdev->tmp_data2, comdev->N_samples);
	    funky_filter(comdev->tmp_data, comdev->tmp_data2 , q, comdev->N_samples);
	    flip_vec(comdev->tmp_data2, comdev->N_samples);
	    add_to_vec(comdev->tmp_data2, u_ch, comdev->N_samples);  // Auf Ausgang addieren

	    if (debug_mode==1) printf("Nach Q Filter ");
	    dump_vec(comdev->tmp_data2, comdev->N_samples);

	  } else if (comdev->filtfiltmode == 2) { // benutze CV-Matrix für Q-Filter
	    //put_zero_vec(tmp_ldata_ch, comdev->N_samples);
	    //tmp_ldata_ch[1] = 1;

	    if (debug_mode==1) printf("Vor Q Filter (CV-Matrix) ");
	    dump_vec(tmp_ldata_ch, comdev->N_samples);

	    double *Q = extract_double_arr_ptr(get_inp_q_copy, index_q, comdev->N_samples*comdev->N_samples);  // richtige Matrix extrahieren  // THRMOD

	    if (debug_mode==1) printf("ILC: Q-Filtering u_%d, -> y_%d\n", i, j);
	    if (debug_mode==1) printf("ILC: Q (index = %d): Q[0..3] = %f, %f, %f, %f\n", index_q, Q[0], Q[1], Q[2], Q[3]);
	    if (debug_mode==1) printf("ILC: Q[1,1]: %f; Q[2,2]: %f\n", sqmatrix_element(Q, 1, 1, comdev->N_samples), sqmatrix_element(Q, 2, 2, comdev->N_samples));

	    funky_cvmatrix_filter(Q, tmp_ldata_ch, comdev->tmp_data2, comdev->N_samples);
	    add_to_vec(comdev->tmp_data2, u_ch, comdev->N_samples);  // Auf Ausgang addieren

	    if (debug_mode==1) printf("Nach Q Filter (CV-Matrix)");
	    dump_vec(comdev->tmp_data2, comdev->N_samples);
	  }
  }

  if (debug_mode==1) printf("Alte Stelltrajektorie:\n");
  for (i=0; i<comdev->N_samples; ++i) {
	  if (debug_mode==1) printf(" %f, ", comdev->u_it_cp[i]);
  }
  if (debug_mode==1) printf("\n");

  if (debug_mode==1) printf("Neue Stelltrajektorie:\n");
  for (i=0; i<comdev->N_samples; ++i) {
	  if (debug_mode==1) printf(" %f, ", comdev->u_it[i]);
  }
  if (debug_mode==1) printf("\n");

  if (debug_mode==1) printf("gesampelte Daten (y):\n");
  for (i=0; i<comdev->N_samples; ++i) {
	  if (debug_mode==1) printf(" %f, ", comdev->sample_data[i]);
  }
  if (debug_mode==1) printf("\n");

  // Kopie der gesampelten Daten erstellen
  copy_vec(comdev->sample_data, comdev->old_sample_data, comdev->N_samples*comdev->num_channel);




/*
		double *test = extract_double_arr_ptr(comdev->sample_data, 1, comdev->N_samples);
		if (debug_mode==1) printf("Werte des 2ten Kanals: %f, %f, %f\n", test[0], test[1], test[2]);
		double *test2 = (comdev->sample_data + sizeof(double)*comdev->N_samples*1);
		double tmp1 = extract(comdev->sample_data, 1, 0, comdev->N_samples);
		double tmp2 = extract(comdev->sample_data, 1, 1, comdev->N_samples);
		double tmp3 = extract(comdev->sample_data, 1, 2, comdev->N_samples);
		if (debug_mode==1) printf("... %f %f %f\n", tmp1, tmp2, tmp3);
*/
/*
		// Einfach eine Kopie der Aufnahme
		for (i=0; i<comdev->num_channel; ++i) {
			for(j=0; j<comdev->N_samples; ++j) {
				tmp = extract(comdev->sample_data, i, j, comdev->N_samples);
				extract(comdev->u_it, i, j, comdev->N_samples) = tmp;
			}
		}
*/
}

int ilc_bloc_zupdate(scicos_block *block,int flag)
{
  struct ilcDev * comdev = (struct ilcDev *) (*block->work);
  double in;
  int i;

  if (comdev->active == 1 && block->nevprt == 1) { // Werte Aufnehmen
	if (  (comdev->mtot <= comdev->out_counter) 
	       && (comdev->out_counter < (comdev->N_samples+comdev->mtot))) 
	{
		for (i=0; i<comdev->num_channel; ++i) {
		//out = comdev->u_it[i*comdev->N_samples + comdev->out_counter];
		  in = block->inptr[0][i];
		  extract(comdev->sample_data, i, comdev->out_counter-comdev->mtot, comdev->N_samples) = in;
		  if (debug_mode==1) printf("recorded_ch%i: %f\n", i, in);
		}
	} else {
		; //comdev->active = 0; // sampling deaktivieren
	}

	if (comdev->out_counter == (comdev->N_samples+comdev->mtot) - 1) {  // Letzter Wert wurde aufgenommen
	    printf("recorded last sample --> updating ilc\n");

	    int ch, j;

	    for (ch = 0; ch < comdev->num_channel; ++ch) { // jeden referenzkanal kopieren
    	      for (j = 0; j < comdev->N_samples; ++j)
      	      comdev->inptr_ref_cpy[j + ch*comdev->N_samples] = get_reference_input_vec(ch,j);
  	    }		
  	    double *refin_ch2 = get_reference_input_copy_vecptr(1); // Test
            if (debug_mode==1) printf("refin_ch2[2] = %f ==? %f\n", refin_ch2[2], get_reference_input_vec(1,2));


	    //comdev->block = block;
	    //ilc_bloc_ilc_update(comdev);

            pthread_mutex_lock(&comdev->ilcdata_mutex); // Datenbereiche sperren
	    rt_ilc_send_command(comdev, 1);
	}

	++comdev->out_counter;
  }

  float tmp;

  if (comdev->active == 1 && block->nevprt == 2)
  	if (debug_mode==1) printf("ILC: Aktivierung (Event 2), während sampling noch aktiv!\n");

  if (comdev->out_counter >= comdev->mtot + comdev->N_samples) // letzter Wert wurde aufgenommen
	comdev->active = 0;	// Sampling beenden

  if (comdev->active == 0 && block->nevprt == 2) { // sampling aktivieren
	if (comdev->first_run != 1) {
		// neue Stellgröße berechnen
	      //ilc_bloc_ilc_update(block);

	}

	// zu diesem Zeitpunkt sollte die ILC geupdated sein!
	// Ansonsten muss hier gewartet werden
	if (debug_mode==1) printf("ILC: trying to pass ilcdata Mutex\n");
	pthread_mutex_lock(&comdev->ilcdata_mutex);
	pthread_mutex_unlock(&comdev->ilcdata_mutex);

	comdev->out_counter = 0;
	comdev->active = 1;
	comdev->first_run = 0;

	if (debug_mode==1) printf("ILC: Aktiviere sampling\n");
  }




  return 0;
}


int ilc_bloc_outputs(scicos_block *block,int flag)
{
  struct ilcDev * comdev = (struct ilcDev *) (*block->work);
  double out;
  int i;

  if (debug_mode==1) printf("out_counter = %i, N_samples = %i\n", comdev->out_counter,comdev->N_samples);

  if (comdev->active == 1 && comdev->out_counter < comdev->N_samples) {
    for (i=0; i<comdev->num_channel; ++i) {
      //out = comdev->u_it[i*comdev->N_samples + comdev->out_counter];
      out = extract(comdev->u_it, i, comdev->out_counter, comdev->N_samples);
      if (debug_mode==1) printf("out_ch%i: %f\n", i, out);
      block->outptr[0][i]= out;  // u - Ausgang
      block->outptr[1][i] = get_reference_input_vec(i, comdev->out_counter); // Referenz auch ausgeben
    }
  } else {
    for (i=0; i<comdev->num_channel; ++i) {
      	if (comdev->hold_type == 0 || comdev->first_run == 1) {  // Initiale Ausgangswerte
		out = 0;
		block->outptr[0][i]= out;
		if (debug_mode==1) printf("out_ch%i: %f (initial run / holdtype = 0)\n", i, out);
		block->outptr[1][i] = get_reference_input_vec(i, comdev->N_samples-1);  // Bei der Referenz immer in den Pausen den letzten Wert ausgeben
	} else {  // Halte letzten Wert
        	out = extract(comdev->u_it, i, comdev->N_samples-1, comdev->N_samples);
      		if (debug_mode==1) printf("out_ch%i: %f (holding last value)\n", i, out);
      		block->outptr[0][i]= out;
		block->outptr[1][i] = get_reference_input_vec(i, comdev->N_samples-1); // Referenz auch ausgeben
	}

	//set_u_output(i,sample,out)
    }
    //comdev->active = 0; // sampling deaktivieren
  }
  for (i=0; i<comdev->num_channel; ++i) {  // Stellgröße komplett als Vektor herausgeben
	double *out_u = extract_double_arr_ptr(get_output_vecptr(i), 0, comdev->N_samples) ; // Erster Teil des Ausgangsvektors
	double *out_y = extract_double_arr_ptr(get_output_vecptr(i), 1, comdev->N_samples) ; // Zweiter Teil des Ausgangsvektors
	double *u_it_ch = extract_double_arr_ptr(comdev->u_it, i, comdev->N_samples);
	double *sample_data_ch = extract_double_arr_ptr(comdev->old_sample_data, i, comdev->N_samples);

  	copy_vec(u_it_ch, out_u, comdev->N_samples);
	copy_vec(sample_data_ch, out_y, comdev->N_samples);
  }
//  block->outptr[3][0]=4;
  return 0
;}

int ilc_bloc_ending(scicos_block *block,int flag)
{
  struct ilcDev * comdev = (struct ilcDev *) (*block->work);

  rt_ilc_send_command(comdev, 2);
  pthread_join(comdev->thread_ilc_update, NULL);

  pthread_mutex_destroy(&comdev->thread_mutex);
  pthread_mutex_destroy(&comdev->ilcdata_mutex);
  pthread_cond_destroy(&comdev->thread_condition);

  free(comdev->u_it);
  free(comdev->u_it_cp);
  free(comdev->sample_data);
  free(comdev->old_sample_data);
  free(comdev->tmp_data);
  free(comdev->tmp_data2);
  free(comdev->tmp_ldata);


  free(comdev);

  return 0;
}



