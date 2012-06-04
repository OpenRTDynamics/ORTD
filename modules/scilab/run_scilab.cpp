#include "scilab.h"

/*#if !defined(ARRAY_SIZE)
    #define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif*/


run_scilab::run_scilab(const char* scilab_path) // Constructor
{
    this->scilab_path = new char [strlen(scilab_path) + 1];
    if (strcpy(this->scilab_path, scilab_path) == NULL)
    {
        fprintf(stderr, "Error initializing the scilab path!\n");
    }
    //printf("scilab_path = %s\n", this->scilab_path);
    ToChild[0] = 0;
    ToChild[1] = 0;
    ToParent[0] = 0;
    ToParent[1] = 0;
    
    readfd = NULL;
    writefd = NULL;
}

run_scilab::~run_scilab()  // Destructor
{
    delete scilab_path;
    //close pipes
    ToChild[0] = 0;
    ToChild[1] = 0;
    ToParent[0] = 0;
    ToParent[1] = 0;
    if (readfd != NULL) fclose(readfd);
    if (writefd != NULL) fclose(writefd);

    //printf("run_scilab deleted\n");
}

bool run_scilab::init()    // starts scilab and generates pipes to scilab to send and receive data
{
    int status, pid;

    status = pipe(ToChild); // Initialization of the ToChild-pipe
    if (status == -1)
    {
        fprintf(stderr, "Error creating unnamed pipe ToChild!\n");
        return false;
    }

    status = pipe(ToParent);   // Initialization of the ToParent-pipe
    if (status == -1)
    {
        fprintf(stderr, "Error creating unnamed pipe ToParent!\n");
        return false;
    }

    pid = fork();  // Create child process
    if (pid == 0)
    {
        /* child process
         * if fork returns 0, it is the child process
         */

        //printf ("Child process with PID: %d\n", getpid());

        status = close(ToChild[1]);   // the write input must be closed,
        // because the child writes to the ToParent-pipe
        if (status == -1)
        {
            fprintf(stderr, "Error closing write input of pipe ToChild in child process!\n");
            exit (1);
        }
        status = close(ToParent[0]);  // the read output must be closed,
        // because the child reads from the ToChild-pipe
        if (status == -1)
        {
            fprintf(stderr, "Error closing read output of pipe ToParent in child process!\n");
            exit (1);
        }

        status = dup2(ToChild[0],0);  // Replace stdin with the in-side of the ToChild-pipe
        if (status == -1)
        {
            fprintf(stderr, "Error replacing stdin with the in-side of the ToChild-pipe in child process!\n");
            exit (1);
        }
        status = dup2(ToParent[1],1); // Replace stdout with the out-side of the ToParent-pipe
        if (status == -1)
        {
            fprintf(stderr, "Error replacing stdout with the out-side of the ToParent-pipe in child process!\n");
            exit (1);
        }
        
        
        #include "scilabconf.h"

        // run scilab
//         status = execl(scilab_path, "scilab", "-nw", NULL);   // replaces the child process image

//  fprintf(stderr, "**** compare path %d \n", strcmp(scilab_path, "BUILDIN_PATH"));

        if ( strcmp(scilab_path, "BUILDIN_PATH") == 0 ) {
	  fprintf(stderr, "running scilab from buildin path: %s\n", SCILAB_EXEC);
          status = execl(SCILAB_EXEC, "scilab", "-nwni", NULL);   //  no gui, replaces the child process image	  	  
	} else {
	  fprintf(stderr, "running scilab from the given path: %s\n", scilab_path);
          status = execl(scilab_path, "scilab", "-nwni", NULL);   //  no gui, replaces the child process image
	}
	
        // with a new scilab process image
        if (status == -1)
        {
            fprintf(stderr, "Error replacing the child process with a scilab process!\n");
// 	    fprintf(stderr, "Tried to run %s\n", scilab_path);
            exit (1);
        }

        exit (1);
    }
    else if (pid > 0)
    {
        /* parent process
         * if fork returns a positive number, it is the parent process
         * it is the PID of the newly created child process
         */

        //printf ("Parent process with PID: %d\n", getpid());

        status = close(ToChild[0]);   // the read output must be closed,
        // because the parent reads from the ToParent-pipe
        if (status == -1)
        {
            fprintf(stderr, "Error closing read output of pipe ToChild in parent process!\n");
            return false;
        }

        status = close(ToParent[1]);  // the write input must be closed,
        // because the parent write to the ToChild-pipe
        if (status == -1)
        {
            fprintf(stderr, "Error closing write input of pipe ToParent in parent process!\n");
            return false;
        }

        readfd = fdopen(ToParent[0], "r"); // associate the read output of the pipe ToParent
        // to the read stream
        if (readfd == NULL)
        {
            fprintf(stderr, "Error associating the read output of the pipe ToParent to the read stream!\n");
            return false;
        }

        writefd = fdopen(ToChild[1], "w"); // associate the write input of the pipe ToChild
        // to the write stream
        if (writefd == NULL)
        {
            fprintf(stderr, "Error associating the write input of the pipe ToChild to the write stream!\n");
            return false;
        }

    }
    else
    {
        /* If a negative value (-1) is returned, an error has occurred */
        fprintf (stderr, "Error creating child process (PID < 0)\n");
	return false;
//         exit (0);
    }

    return true;
}

bool run_scilab::send_to_scilab(const char* cmd)   // send command to scilab
{
    int number_of_bytes;
    number_of_bytes = fprintf(writefd, "%s\n", cmd);  // write the command to the write stream of scilab
    if (number_of_bytes < 0)
    {
        fprintf(stderr, "Error writing the command: %s to scilab!\n", cmd);
        return false;
    }
    fflush(writefd);

//    fprintf(stderr, ">>> %s\n", cmd);

    return true;
}

bool run_scilab::send_to_scilab_noFlush(const char* cmd)   // send command to scilab
{
    int number_of_bytes;
    number_of_bytes = fprintf(writefd, "%s\n", cmd);  // write the command to the write stream of scilab
    if (number_of_bytes < 0)
    {
        fprintf(stderr, "Error writing the command: %s to scilab!\n", cmd);
        return false;
    }
//     fflush(writefd);

    return true;
}


FILE* run_scilab::get_readfd()   // read from scilab
{
    return readfd; // return the read stream of scilab
}




scilab_calculation::scilab_calculation(const char *scilab_path, char *init_cmd, char *destr_cmd, char *calc_cmd)//, int Nin, int Nout, int *insizes, int *outsizes, double **inptr, double **outptr)
{
    bool status;

    scilab = new run_scilab(scilab_path);

    status = scilab->init();
    if (status == false)
    {
        fprintf(stderr, "scilab_interf: Error init a scilab instance!\n");
	delete scilab;
	
        exit(0);
    }

    this->init_cmd = init_cmd;
    this->destr_cmd = destr_cmd;
    this->calc_cmd = calc_cmd;

}

scilab_calculation::~scilab_calculation()  // Destructor
{
    bool status;

    status = scilab->send_to_scilab(destr_cmd);
    if (status == false)
    {
        fprintf(stderr, "Error writing to scilab!\n");
    }

    exit_scilab_and_read_remaining_data();

    init_cmd = "";
    destr_cmd == "";
    calc_cmd == "";

    delete scilab;
    //printf("scilab_calculation deleted\n");
}

bool scilab_calculation::init()
{
    bool status;

    status = scilab->send_to_scilab(init_cmd);
    if (status == false)
    {
        fprintf(stderr, "Error writing to scilab!\n");
        return false;
    }

    return true;
}


bool scilab_calculation::send_vector_to_scilab(int vector_nr, double *data, int veclen)
{
    bool status;
    char tmp[1000];
    int i;
    char buf[1024];
    FILE *read_fd = scilab->get_readfd();

    sprintf(tmp, "scilab_interf.invec%d = [ ", vector_nr);

    status = scilab->send_to_scilab_noFlush(tmp);
    if (status == false)
    {
        fprintf(stderr, "scilab_interface: Error writing to scilab!\n");
        return false;
    }

    for (i = 0; i < veclen; ++i) {
        sprintf(tmp, "%f", data[i]);
        status = scilab->send_to_scilab_noFlush(tmp);
        if (status == false)
        {
            fprintf(stderr, "scilab_interface: Error writing to scilab!\n");
            return false;
        }
    }

    status = scilab->send_to_scilab("];"); // now also flush the data
    if (status == false)
    {
        fprintf(stderr, "scilab_interface: Error writing to scilab!\n");
        return false;
    }

    return true;
}

bool scilab_calculation::calculate(int invec_no, int outvec_no, int insize, int outsize)
{
    bool status;
    char buf[1024];
    int state = 0;
    int rec_vec_no, rec_veclen;

    status = scilab->send_to_scilab(calc_cmd);
    if (status == false)
    {
        fprintf(stderr, "scilab_interface: Error writing to scilab!\n");
        return false;
    }

//     FILE *read_fd = scilab->get_readfd();
// 
//     status = scilab->send_to_scilab("printf(\"\\n\")"); // to get something to read
//     if (status == false)
//     {
//         fprintf(stderr, "Error writing to scilab!\n");
//         return false;
//     }


    return true;
/*    
    do
    {
        if (!feof(read_fd)
                && !ferror(read_fd)
                && fgets(buf, sizeof buf, read_fd) != NULL)
        {
            if (state==0) { 
	      fprintf(stderr, "scilab says: >>> %s\n", buf); 	      
	    } else {
	      fprintf(stderr, "REMscilab says: >>> %s\n", buf); 	      
	    }

            if (strstr (buf,"scilab_interf  =") != NULL)      {
                state = 1;
            }         else if (strstr (buf,"error") != NULL)         {
                state = -1;
            }
        }
        else
        {
            fprintf(stderr, "scilab_interf: error: nothing to read in check-routine (state 0) of calculate\n");
            return false;
        }
    } while (state == 0);

    if (state == 1)
    {
        do
        {
            if (!feof(read_fd)
                    && !ferror(read_fd)
                    && fgets(buf, sizeof buf, read_fd) != NULL)
            {
//                 fprintf(stderr, "scilab says: >>> %s\n", buf);

                if (strstr (buf,"invec") != NULL)
                {
                    rec_vec_no = -999;
                    sscanf (buf,"%*5s%d%*1s %*1s%d", &rec_vec_no, &rec_veclen);
                    if (rec_vec_no == -999)
                    {
                        fprintf(stderr, "scilab_interf: Error: invector number could not be read!\n");
                        return false;
                    }
                    else if (rec_vec_no != invec_no)
                    {
                        fprintf(stderr, "scilab_interf: Error: Wrong invector number received (received = %d, expected = %d)!\n", rec_vec_no, invec_no);
                        return false;
                    }
                    else
                    {
                        if (rec_veclen != insize)
                        {
                            fprintf(stderr, "scilab_interf: Error: Wrong invector length received (received = %d, expected = %d)!\n", rec_veclen, insize);
                            return false;
                        }
                        else
                        {
                            state = 2;
                        }
                    }
                }
            }
            else
            {
                fprintf(stderr, "scilab_interf: error: nothing to read in check-routine (state 1) of calculate\n");
                return false;
            }
        } while (state == 1);
    }
    else if (state == -1)
    {
        if (!feof(read_fd)
                && !ferror(read_fd)
                && fgets(buf, sizeof buf, read_fd) != NULL)
        {
            fprintf(stderr, "error: %s\n", buf);
            return false;
        }
        else
        {
            fprintf(stderr, "scilab_interf: error: nothing to read (error-message of scilab missing in calculate)\n");
            return false;
        }
    }

    if (state == 2)
    {
        do
        {
            if (!feof(read_fd)
                    && !ferror(read_fd)
                    && fgets(buf, sizeof buf, read_fd) != NULL)
            {
                fprintf(stderr, "REMscilab says: >>> %s\n", buf);

                if (strstr (buf,"outvec") != NULL)
                {
                    rec_vec_no = -999;
                    sscanf (buf,"%*6s%d%*1s %*1s%d", &rec_vec_no, &rec_veclen);
                    if (rec_vec_no == -999)
                    {
                        fprintf(stderr, "scilab_interf: Error: outvector number could not be read!\n");
                        return false;
                    }
                    else if (rec_vec_no != outvec_no)
                    {
                        fprintf(stderr, "scilab_interf: Error: Wrong outvector number received (received = %d, expected = %d)!\n", rec_vec_no, outvec_no);
                        return false;
                    }
                    else
                    {
                        if (rec_veclen != outsize)
                        {
                            fprintf(stderr, "scilab_interf: Error: Wrong outvector length received (received = %d, expected = %d)!\n", rec_veclen, outsize);
                            return false;
                        }
                        else
                        {
                            state = 3;
                        }
                    }
                }
            }
            else
            {
                fprintf(stderr, "scilab_interf: error: nothing to read in check-routine (state 2) of calculate\n");
                return false;
            }
        } while (state == 2);
    }

    if (state == 3)
    {
        return true;
    }
    else
    {
        fprintf(stderr, "scilab_interf: unknown error in calculate\n");
        return false;
    }*/
}

bool scilab_calculation::read_vector_from_scilab(int vector_nr, double *data, int veclen)
{
    bool status;
    char tmp[1000];
    char buf[1024];
    int rec_veclen = -1;
    int i;
    int state = 0;

    FILE *read_fd = scilab->get_readfd();

//     fprintf(stderr, "Trying to get vector from scilab\n");

//     sprintf(tmp, "scilab_interf.outvec%d", vector_nr);
//     status = scilab->send_to_scilab(tmp);
//     if (status == false)
//     {
//         fprintf(stderr, "Error writing to scilab!\n");
//         return false;
//     }

//     status = scilab->send_to_scilab("printf(\"\\n\")"); // to get something to read
//     if (status == false)
//     {
//         fprintf(stderr, "Error writing to scilab!\n");
//         return false;
//     }

    // Make scilab to print out a "magic" number which helps to separate from the usual output of the scilab calculation
    sprintf(tmp, "try;"
                 "printf(\"vlg3hdl1289fn28=%%d \\n \", length(scilab_interf.outvec%d));  "
		  "catch; "
		  "printf(\"ERRORvlg3hdl1289fn28\\n \" ); end; " , vector_nr);
    status = scilab->send_to_scilab(tmp);
    if (status == false)
    {
        fprintf(stderr, "scilab_interface: Error writing to scilab!\n");
        return false;
    }

    do
    {
        if (!feof(read_fd)
                && !ferror(read_fd)
                && fgets(buf, sizeof buf, read_fd) != NULL)
        {
	    if (sscanf (buf,"vlg3hdl1289fn28=%d",&rec_veclen) == 1) { // is there my magic number?
	        // check vector length
                if (rec_veclen != veclen)   {
                    fprintf(stderr, "scilab_interface: Error: Wrong vector length received! Expected length %d but got %d\n", veclen, rec_veclen);
                    return false;
                }  else  {
//                  fprintf(stderr, "got veclen=%d\n", rec_veclen);
                    state = 1;
                }
	      
	    } else if (strstr (buf,"ERRORvlg3hdl1289fn28") != NULL)   { // insted got error FIXME: the normal code could also hav send this
                state = -1;
		fprintf(stderr, "scilab_interface: There was an error reading out the variable \"scilab_interf.outvec%d\" \n", vector_nr);
		return false;
            } else if (state == 0) { // as long as "vectorlen_g3hdl1289fn28" or "error" is not found, put out the scilab outputs to stderr
	      fprintf(stderr, "scilab says: >>> %s\n", buf);
	    }


	    
// 	    if (strstr (buf,"vectorleng3hdl1289fn28=") != NULL) // wait for the special line
//             {
//                 sscanf (buf,"%10s%d",tmp,&rec_veclen);
//                 if (rec_veclen != veclen)   {
//                     fprintf(stderr, "Error: Wrong vector length received!\n");
//                     return false;
//                 }  else  {
// //                  fprintf(stderr, "got veclen=%d\n", rec_veclen);
//                     state = 1;
//                 }
//                 
//                 
//             }  else if (strstr (buf,"error") != NULL)   {
//                 state = -1;
//             }
//             
        }
        else
        {
            fprintf(stderr, "scilab_interface: error: nothing to read in vectorlen-check-routine (state 0) of read_vector_from_scilab\n");
            return false;
        }
    } while (state == 0);


    if (state == 1)
    {
        if (print_vector(vector_nr) == false)
        {
            fprintf(stderr, "scilab_interface: Error printing vector!\n");
            return false;
        }
//      fprintf(stderr, "printed\n");

        for (i = 0; i < veclen; i++)
        {
            if (!feof(read_fd)
                    && !ferror(read_fd)
                    && fgets(buf, sizeof buf, read_fd) != NULL)
            {
//                 fprintf(stderr, "scilab says: >>> %s\n", buf);
                if (((int)buf[0] != 32) && ((int)buf[0] != 10) && ((int)buf[3] != 0)) // catch empty lines
                {
                    sscanf (buf,"%lf",&data[i]);
                }
                else
                {
                    i--;
                }
            }
            else
            {
                fprintf(stderr, "scilab_interface: error: vector not possible to read in read_vector_from_scilab (state 1)\n");
                return false;
            }
        }
    }
//     else if (state == -1) // in case of an error read out the whole buffer
//     {
//         if (!feof(read_fd)
//                 && !ferror(read_fd)
//                 && fgets(buf, sizeof buf, read_fd) != NULL)
//         {
//             fprintf(stderr, "error: %s\n", buf);
//             return false;
//         }
//         else
//         {
//             fprintf(stderr, "error: nothing to read (error-message of scilab missing in read_vector_from_scilab)\n");
//             return false;
//         }
//     }

    return true;
}

bool scilab_calculation::print_vector(int vector_nr)
{
    bool status;
    char tmp[1000];

    //
    sprintf(tmp, "for val=scilab_interf.outvec%d\n printf(\"%%f\\n\", val) \n end", vector_nr);

    status = scilab->send_to_scilab(tmp);
    if (status == false)
    {
        fprintf(stderr, "Error writing to scilab!\n");
        return false;
    }

    return true;
}

bool scilab_calculation::exit_scilab_and_read_remaining_data()
{
    bool status;
    char buf[1024];
        

    status = scilab->send_to_scilab("exit\nquit\n");
    if (status == false)
    {
        fprintf(stderr, "Error writing to scilab!\n");
        return false;
    }

    FILE *read_fd = scilab->get_readfd();

    while (!feof(read_fd)
            && !ferror(read_fd)
            && fgets(buf, sizeof buf, read_fd) != NULL)
    {
        fprintf(stderr, "scilab says: >>> %s\n", buf);      
	
        // Was ist das hier? - leere Zeilen abfangen
        if (((int)buf[0] != 32) && ((int)buf[0] != 10) && ((int)buf[3] != 0))
        {
//          printf("--->%s<---\n", buf);
        }
    }

    fprintf(stderr, "Scilab exited\n");

    return true;
}
