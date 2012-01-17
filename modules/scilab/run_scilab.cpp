#include "scilab.h"



/*

TODO: -- Fehler und printf von scilab ausgeben
      -- Wenn Scilab Binaray ein Link ist funktioniert es nicht
      -- verschiedenen default angaben für das scilab binary probieren
      -- nicht höngen bleiben falls vektorgröße falsch
      -- auch sonst nicht hängen bleiben
      -- Schleife vereinfachen











*/


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
}

run_scilab::~run_scilab()  // Destructor
{
   delete scilab_path;
   //close pipes
   ToChild[0] = 0;
   ToChild[1] = 0;
   ToParent[0] = 0;
   ToParent[1] = 0;
   fclose(readfd);
   fclose(writefd);
   
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
         return false;
      }
      status = close(ToParent[0]);  // the read output must be closed,
                                          // because the child reads from the ToChild-pipe
      if (status == -1)
      {
         fprintf(stderr, "Error closing read output of pipe ToParent in child process!\n");
         return false;
      }
      
      status = dup2(ToChild[0],0);  // Replace stdin with the in-side of the ToChild-pipe
      if (status == -1)
      {
         fprintf(stderr, "Error replacing stdin with the in-side of the ToChild-pipe in child process!\n");
         return false;
      }
      status = dup2(ToParent[1],1); // Replace stdout with the out-side of the ToParent-pipe
      if (status == -1)
      {
         fprintf(stderr, "Error replacing stdout with the out-side of the ToParent-pipe in child process!\n");
         return false;
      }
      
      status = execl(scilab_path, "scilab", "-nw", NULL);   // replaces the child process image
                                                                  // with a new scilab process image
      if (status == -1)
      {
         fprintf(stderr, "Error replacing the child process with a scilab process!\n");
         return false;
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
      exit (0);
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
      fprintf(stderr, "Error init a scilab instance!\n");
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
   
   sprintf(tmp, "scilab_interf.invec%d = [ ", vector_nr);
   
   status = scilab->send_to_scilab(tmp);
   if (status == false)
   {
      fprintf(stderr, "Error writing to scilab!\n");
      return false;
   }
   
   for (i = 0; i < veclen; ++i) {
      sprintf(tmp, "%f", data[i]);
      status = scilab->send_to_scilab(tmp);
      if (status == false)
      {
         fprintf(stderr, "Error writing to scilab!\n");
         return false;
      }
   }
   
   status = scilab->send_to_scilab("];");
   if (status == false)
   {
      fprintf(stderr, "Error writing to scilab!\n");
      return false;
   }
   
   return true;
}

bool scilab_calculation::calculate()
{
   bool status;
   
   status = scilab->send_to_scilab(calc_cmd);
   if (status == false)
   {
      fprintf(stderr, "Error writing to scilab!\n");
      return false;
   }
   
   return true;
}

bool scilab_calculation::read_vector_from_scilab(int vector_nr, double *data, int veclen)
{
   bool status;
   char tmp[1000];
   char buf[1024];
   int rec_veclen;
   int i;

   FILE *read_fd = scilab->get_readfd(); 
   
   printf("Trying to get vector from scilab\n");

//    sprintf(tmp, "printf(\"Hallo \\n  \" ); ");
//    status = scilab->send_to_scilab(tmp);
// 
//    sprintf(tmp, "printf(\"Hallo2 \\n  \" ); ");
//    status = scilab->send_to_scilab(tmp);

   
 // TODO
   // Würde eher vermuten, dass das printf hier einen Zeilenumbruch benötigt. Wieso nicht? Möglicherweise beide KOmmandos an scilab (dieses und den vector ausgeben zusammenfassen
//  sprintf(tmp, "printf(\"vectorlen=%%d \\n \", length(scilab_interf.outvec%d));  ", vector_nr);
   
   sprintf(tmp, "printf(\"vectorlen=%%d  \", length(scilab_interf.outvec%d));  ", vector_nr);
   status = scilab->send_to_scilab(tmp);
   if (status == false)
   {
      fprintf(stderr, "Error writing to scilab!\n");
      return false;
   }
   if (print_vector(vector_nr) == false)
   {
      fprintf(stderr, "Error printing vector!\n");
      return false;
   }
   
/*   while (
          
          fgets(buf, sizeof buf, read_fd) != NULL) {

     	 printf("scilab: >>> %s\n", buf);
     
	 if (strstr (buf,"vectorlen=") != NULL)
         {
	    printf("got veclen\n");
	     
            sscanf (buf,"%10s%d",tmp,&rec_veclen);
            if (rec_veclen != veclen)
            {
               fprintf(stderr, "Error: Wrong vector length received!\n");
               return false;
            }
	 }
              
   }

printf("ok\n");*/
   
   // TODO Schleife echt kompliziert
   // Eine Schleife für zwei Sachen --> 2 Schleifen für ...
   do 
   {
      if (!feof(read_fd)
          && !ferror(read_fd)
          && fgets(buf, sizeof buf, read_fd) != NULL)
      {
 	 fprintf(stderr, "scilab says: >>> %s\n", buf);

	 if (strstr (buf,"vectorlen=") != NULL)
         {
	     
            sscanf (buf,"%10s%d",tmp,&rec_veclen);
            if (rec_veclen != veclen)
            {
               fprintf(stderr, "Error: Wrong vector length received!\n");
               return false;
            }
            else
            {
//        	        printf("got veclen=%d\n", rec_veclen);

               for (i = 0; i < veclen; i++)
               {
                  if (!feof(read_fd)
                     && !ferror(read_fd)
                     && fgets(buf, sizeof buf, read_fd) != NULL)
                  {
                	 fprintf(stderr, "scilab says: >>> %s\n", buf);

                     sscanf (buf,"%lf",&data[i]);
                  }
               }
            }
         }
      }
   } while ((strstr (buf,"vectorlen=") == NULL) && (i < veclen));
   
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
     // Was ist das hier?
      if (((int)buf[0] != 32) && ((int)buf[0] != 10) && ((int)buf[3] != 0))
      {
//          printf("--->%s<---\n", buf);
      }
   }
   
   fprintf(stderr, "Scilab exited\n");
   
   return true;
}
