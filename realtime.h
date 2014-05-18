
/*
    Copyright (C) 2012, 2013  Christian Klauer

    This file is part of OpenRTDynamics, the Real-Time Dynamics Framework

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




#ifndef _ORTD_REALTIME_H
#define _ORTD_REALTIME_H 1

#include <pthread.h>

struct TaskPriority_t {
  int *par;
  int Npar;
};

int ortd_rt_SetThreadProperties2(struct TaskPriority_t TaskPriority);
int ortd_rt_SetThreadProperties(int *par, int Npar);
int ortd_rt_SetCore(int core_id);
long int ortd_mu_time();
int ortd_rt_ChangePriority(unsigned int flags, int priority);
int ortd_pthread_cancel(pthread_t thread);


#define ORTD_RT_REALTIMETASK 1
#define ORTD_RT_NORMALTASK 2


#endif
