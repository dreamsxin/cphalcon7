#ifndef PHPSCI_EXT_GPU_H
#define PHPSCI_EXT_GPU_H

#include "config.h"

#ifdef HAVE_CLBLAS
#include "clBLAS.h"

void start_clblas_context();
cl_command_queue getCLQueue();
cl_context getCLContext();
#endif


#endif //PHPSCI_EXT_GPU_H
