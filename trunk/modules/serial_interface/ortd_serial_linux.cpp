/**-----------------------------------------------------------------------------------
 *  Copyright (C) 2003  Nils-Otto Neg�rd Max Planck Institute for Dynamic sof Complex Dynamical Systems
 *  Copyright (C) 2008  Holger Nahrstaedt
 *  Copyright (C) 2011  Christian Klauer
 *
 *  This file is part of HART, the Hardware Access in Real Time Toolbox for Scilab/Scicos.
 *
 *  HART is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  HART is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with HART; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *--------------------------------------------------------------------------------- */


/*!
\class ortd_SerialPort
\brief Serial interface class
\version 1.0
\author Nils-Otto Neg�rd, Henrik Gollee

\warning This code is published under the GNU general public
licence GPL, which means that the code distributed here comes with no
warranty. For more details read the hole licence in the file COPYING.

*/

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include "ortd_serial_linux.h"
#include <errno.h>
#include <linux/serial.h>

// extern long int mu_time();

extern int errno;

/*!
\fn ortd_SerialPort::ortd_SerialPort()
Constructor
*/
ortd_SerialPort::ortd_SerialPort()
{
}

/*!
\fn ortd_SerialPort::~ortd_SerialPort()
Destructor
*/
ortd_SerialPort::~ortd_SerialPort()
{
}

/*!
\fn int ortd_SerialPort::serial_openport(char *port)
\param *port Name of port

Opens port and saves the file descriptor in the object.

Error code:
\verbatim
Error                  Explanation
----------------------------------------------------
0                      No error has occurred
1                      Error opening serial port
\endverbatim

*/
int ortd_SerialPort::serial_openport(char *port)
{
    fd = open(port, O_RDWR | O_NOCTTY); // | O_NOCTTY //O_NDELAY

    if (fd < 0)
    {
        /* arithmetic test because the fd is regarded
        	      * as a number */
        perror("ortd_SerialPort::serial_openport() Error: Invalid int while opening port");
    }

    return fd;
}

int ortd_SerialPort::serial_set_parity(short int parity)
{
    int retval;
    struct termios tio;

    retval = tcgetattr(fd, &tio);

    if (retval < 0)
    {
        perror("ortd_SerialPort::serial_set_parity() Error getting COM state: ");
    }
    else
    {
		//set parity
		// 0 - none
		// 1   odd
		// 2   even

		tio.c_cflag &= ~(PARENB | PARODD);
		if (parity == 2) //parety even
			tio.c_cflag |= PARENB;
		else if (parity== 1) //odd
			tio.c_cflag |= (PARENB | PARODD);

		retval = tcsetattr(fd, TCSANOW, &tio);

		if (retval < 0)
		{
			perror("ortd_SerialPort::serial_openport() Error setting serial port state: ");
		}
    }

    return retval;
}

/*!
\fn int ortd_SerialPort::serial_setupport(long Baud_Rate)

\param Baud_Rate The baud rate for the communication.

Setting up the serial port with the baud-rate given as parameter.

Error code:
\verbatim
Error                  Explanation
----------------------------------------------------
0                      No error has occurred
1                      Error setting up serial port
\endverbatim
*/

int ortd_SerialPort::serial_setupport(long BaudRate, unsigned char HandShake, unsigned char StopBits)
{
    struct termios tio;

    int retval;
    long BAUD;
    /* Get the current configuration. */
    retval = tcgetattr(fd, &tio);
    if (retval < 0)
    {
        perror("ortd_SerialPort::serial_setupport() Error getting COM state: ");
    }
    else
    {
		/*
		 * Fill in the default values
		 */
		memset(&tio, 0, sizeof(tio));

		switch (BaudRate)
		{
		case 460800:
			BAUD = B460800;
			break;
		case 230400:
			BAUD = B230400;
			break;
		case 115200:
			BAUD = B115200;
			break;
		case 57600:
		default:
			BAUD = B57600;
			break;
		case 38400:
			BAUD = B38400;
			break;
		case 19200:
			BAUD  = B19200;
			break;
		case 9600:
			BAUD  = B9600;
			break;
		}  //end of switch baud_rate

		cfsetospeed(&tio,(speed_t)BAUD);
		cfsetispeed(&tio,(speed_t)BAUD);

		tio.c_cflag = BAUD | CS8 | CLOCAL | CREAD;

		//hw handshake
		if (HandShake==1)
			tio.c_cflag = tio.c_cflag | CRTSCTS;
		else
			tio.c_cflag &= ~CRTSCTS;

		if (StopBits == 1)
			tio.c_cflag = tio.c_cflag |CSTOPB;
		else
			tio.c_cflag &= ~CSTOPB;

		tio.c_iflag = IGNPAR | IGNBRK;
		tio.c_oflag = 0;
		tio.c_lflag = 0;
		tio.c_cc[VTIME] = 10/100;//1
		tio.c_cc[VMIN] = 1;//60

		tcflush(fd, TCIOFLUSH);

		retval = tcsetattr(fd, TCSANOW, &tio);
		if (retval < 0)
		{
			perror("ortd_SerialPort::serial_setupport() Error setting serial port state: ");
		}
    }

    return retval;
}


/*!
\fn int ortd_SerialPort::serial_closeport()

Closing the serial port.

Error code:
\verbatim
Error                  Explanation
--------------------------------------------------
0                      No error has occurred
1                      Error closing serial port
\endverbatim
*/
int ortd_SerialPort::serial_closeport()
{
    int retval;

    retval = close(fd);

    if (retval < 0)
    {
        perror("ortd_SerialPort::serial_closeport() Error closing serial port: ");
    }

    return retval;
}


/*!
\fn int ortd_SerialPort::serial_sendstring(const char *buffer, size_t nb)

\param *buffer String of chars to be sent to the serial port.
\param nb Number of bytes to be sent to the serial port.

This function is sending a bytes to the serial port.

Error code:
\verbatim
Error                  Explanation
------------------------------------------------------
0                      No error has occurred
1                      Error writing to the serial port
\endverbatim
*/
int ortd_SerialPort::serial_sendstring(const char *buffer, size_t nb)
{
    int retval;

    retval = write(fd, (const void *) buffer, nb);

    if (retval < 0)
    {
        perror("ortd_SerialPort::serial_sendstring() Error writing to serial port [sendstring]: ");
    }
    return retval;
}


/*!
\fn  int ortd_SerialPort::serial_readstring(char *buffer, size_t nb)

\param *buffer String read from serial port
\param nb Number of bytes to be read from serial port.

This function is reading a string of the length nb from the serial port.

Error code:
\verbatim
Error                  Explanation
----------------------------------------------------
0                      No error has occurred
1                      Error reading serial port
\endverbatim
*/
int ortd_SerialPort::serial_read_timeout(char *buffer, size_t nb)
{
    fd_set rfds;
    struct timeval tv;
    int input_available;
    int flags;
    size_t chrs_read;
    int still_to_read,total_read,time1,time2;

    if( (flags = fcntl(fd, F_GETFL, 0)) < 0)
        printf("ERROR");


    /*
     * O_NONBLOCK flag loeschen und file-status-flags neu setzen
     */

    if(fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0)
        printf("ERROR");

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    /* Wait up to one seconds. */
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    chrs_read=0;
    total_read=0;
    still_to_read=nb;
    while ((total_read<nb)&&(select(fd + 1, &rfds, NULL, NULL, &tv) == 1))
    {
        chrs_read = read(fd, (void *) &buffer[total_read], still_to_read);
        total_read=total_read+chrs_read;
        still_to_read=still_to_read-chrs_read;
        if (chrs_read < 0)
        {
            perror("Error reading from serial port: ");
            total_read=-1;
        }
    }
    return total_read;
}


/*
\fn  int ortd_SerialPort::serial_read(char *buffer, size_t nb)

\param *buffer String read from serial port
\param nb Number of bytes to be read from serial port.

This function is reading a string of the length nb from the serial port.

Error code:
\verbatim
Error                  Explanation
----------------------------------------------------
0                      No error has occurred
1                      Error reading serial port
\endverbatim
*/
int ortd_SerialPort::serial_read_nonblocking(char *buffer, size_t nb)
{
    size_t chrs_read;
    fcntl(fd,F_SETFL,O_NONBLOCK);
    chrs_read = read(fd, (void *) buffer, nb);
    return chrs_read;
}

/*!
\fn  int ortd_SerialPort::serial_readstring(char *buffer, size_t nb)

\param *buffer String read from serial port
\param nb Number of bytes to be read from serial port.

This function is reading a string of the length nb from the serial port.

Error code:
\verbatim
Error                  Explanation
----------------------------------------------------
0                      No error has occurred
1                      Error reading serial port
\endverbatim
*/
int ortd_SerialPort::serial_readstring(char *buffer, size_t nb)
{
    fd_set rfds;
    struct timeval tv;
    int input_available;

    size_t chrs_read;

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    /* Wait up to five seconds. */
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    input_available = select(fd + 1, &rfds, NULL, NULL, &tv);
    //input_available=1;
    /* Don't rely on the value of tv now! */
    if (input_available)
    {
        chrs_read = read(fd, (void *) buffer, nb);
        if (chrs_read < 0)
        {
            perror("Error reading from serial port: ");
        }
        return chrs_read;
    }
    else
    {
        return 0;
    }

}

/*!
\fn  int ortd_SerialPort::serial_flush()

This function is flushing all buffers.

Error code:
\verbatim
Error                  Explanation
----------------------------------------------------
0                      No error has occurred
1                      Error reading serial port
\endverbatim
*/
int ortd_SerialPort::serial_flush()
{
    tcflush(fd, TCIOFLUSH);
    return 0;
}

/*!
\fn  int ortd_SerialPort::serial_toggleDTR()

This function is toggling the DTR-signal.

Error code:
\verbatim
Error                  Explanation
----------------------------------------------------
0                      No error has occurred
1                      Error
\endverbatim
*/
int ortd_SerialPort::serial_toggleDTR()
{
    int status;
    ioctl(fd, TIOCMGET, &status);
    status^=TIOCM_DTR;
    ioctl(fd, TIOCMSET, &status);
    return 0;
}


int ortd_SerialPort::serial_setRTSlow()
{
    int status;
    ioctl(fd, TIOCMGET, &status);
    status &= ~TIOCM_RTS;
    ioctl(fd, TIOCMSET, &status);
    return 0;
}

int ortd_SerialPort::serial_setRTShigh()
{
    int status;
    ioctl(fd, TIOCMGET, &status);
    status &= ~TIOCM_RTS;
    ioctl(fd, TIOCMSET, &status);
    return 0;
}

int ortd_SerialPort::serial_setCTSlow()
{
    int status;
    ioctl(fd, TIOCMGET, &status);
    status &= ~TIOCM_CTS;
    ioctl(fd, TIOCMSET, &status);
    return 0;
}

int ortd_SerialPort::serial_setCTShigh()
{
    int status;
    ioctl(fd, TIOCMGET, &status);
    status |= TIOCM_CTS;
    ioctl(fd, TIOCMSET, &status);
    return 0;
}


/*
 * Additional Buffered IO
*/

int ortd_SerialPort::serial_B_useBufferedIO()
{
  this->BufferedFD = fdopen(fd, "r+"); 
  
//   printf("buffered FILE pre %p\n", this->BufferedFD);
}

int ortd_SerialPort::serial_B_readln(char *data , size_t maxnb)
{
    char * rvp = fgets( data, maxnb, BufferedFD);
    if (NULL == rvp) {
      return -1;
    }
    
//     return rvp;
    return 0;

}

int ortd_SerialPort::serial_B_writeln(char* data)
{
  int ret;

  ret = fputs((char*) data, BufferedFD);  

  if (ret < 0) {
    return -1;
  }
  
  return 1;

}


int ortd_SerialPort::serial_B_BufferFLush()
{
  fflush(this->BufferedFD);

}

int ortd_SerialPort::serial_B_close()
{
  fclose(this->BufferedFD);
}


