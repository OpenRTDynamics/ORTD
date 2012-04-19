#include "ortd_fork.h"
#include <signal.h>

ortd_fork::ortd_fork(char* exec_path, char* chpwd, int prio, bool replace_io) // Constructor
{
    this->replace_io = replace_io;

    this->exec_path = new char [strlen(exec_path) + 1];
    strcpy(this->exec_path, exec_path);
    
    this->chpwdpath = new char [strlen(chpwd) + 1];
    strcpy(this->chpwdpath, chpwd);
    
    fprintf(stderr, "execpath = %s chpwd = %s\n", this->exec_path, this->chpwdpath);
    
    
    ToChild[0] = 0;
    ToChild[1] = 0;
    ToParent[0] = 0;
    ToParent[1] = 0;
}

ortd_fork::~ortd_fork()  // Destructor
{
    delete exec_path;
    delete chpwdpath;
    
    if (replace_io) {
    //close pipes
    ToChild[0] = 0;
    ToChild[1] = 0;
    ToParent[0] = 0;
    ToParent[1] = 0;
    fclose(readfd);
    fclose(writefd);
    }

    //printf("run_scilab deleted\n");
}

bool ortd_fork::init()    // starts scilab and generates pipes to scilab to send and receive data
{
    int status;

    if (replace_io) {
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
    }

    pid = fork();  // Create child process
    if (pid == 0)
    {
        /* child process
         * if fork returns 0, it is the child process
         */

//         printf ("Child process with PID: %d\n", getpid());

        chdir(this->chpwdpath);

        if (replace_io) {
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
        }

//         fprintf(stderr, "exec of %s\n", exec_path);

        status = execlp(exec_path, "",  NULL);   // replaces the child process image
        // with a new scilab process image
        if (status == -1)
        {
            fprintf(stderr, "Error replacing the child process with a process!\n");
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
        if (replace_io) {

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

    }
    else
    {
        /* If a negative value (-1) is returned, an error has occurred */
        fprintf (stderr, "Error creating child process (PID < 0)\n");
        exit (0);
    }

    return true;
}

bool ortd_fork::terminate(int sig)
{
  kill(this->pid, sig);
}


bool ortd_fork::send_to_stdin(const char* cmd)   // send command to scilab
{
    if (replace_io) {
        int number_of_bytes;
        number_of_bytes = fprintf(writefd, "%s\n", cmd);  // write the command to the write stream of scilab
        if (number_of_bytes < 0)
        {
            fprintf(stderr, "Error writing the command: %s to process!\n", cmd);
            return false;
        }
        fflush(writefd);

//    fprintf(stderr, ">>> %s\n", cmd);

        return true;
    }

    return false;
}

FILE* ortd_fork::get_readfd()   // read from scilab
{
    return readfd; // return the read stream of scilab
}


