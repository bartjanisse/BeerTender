
#ifndef _UTILS_H_
#define _UTILS_H_

#include <string.h>		// for memset()
#include <sched.h>		// To set the priority on linux

/**
 *  \brief Macro to print an error message to the console
 */
#define P_ERR(msg) printf("ERROR: %s. In function %s line %d in file %s\n", msg, __func__, __LINE__, __FILE__); 

/**
 *  \brief Function to set priority level of the program
 */
int SetProgramPriority(int priorityLevel);

#endif




