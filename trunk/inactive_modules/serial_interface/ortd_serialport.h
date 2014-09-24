/**-----------------------------------------------------------------------------------
 *  Copyright (C) 2003  Nils-Otto Neg�rd Max Planck Institute for Dynamic sof Complex Dynamical Systems
 *  Copyright (C) 2008  Holger Nahrstaedt 
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





#ifndef _QEXTSERIALPORT_H_
#define _QEXTSERIALPORT_H_

/*POSIX CODE*/
#ifdef _TTY_POSIX_
#include "serial_linux.h"
//#define SerialPort serialport

/*MS WINDOWS CODE*/
#else
#include "serial_windows.h"
#define SerialPort serial_windows
#endif


#endif
