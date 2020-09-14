#include "config.h"

#ifdef HAVE_CLBLAS
#include "gpu.h"
#include "clBLAS.h"

cl_context ctx;
cl_command_queue queue;

void
start_clblas_context() {
    cl_context_properties props[3] = { CL_CONTEXT_PLATFORM, 0, 0 };
    cl_platform_id platform = 0;
    cl_device_id device = 0;
    cl_int err;


    /* Setup OpenCL environment. */
    err = clGetPlatformIDs( 1, &platform, NULL );
        err = clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL );

    props[1] = (cl_context_properties)platform;

    ctx = clCreateContext( props, 1, &device, NULL, NULL, &err );
    queue = clCreateCommandQueue( ctx, device, 0, &err );

    /* Setup clBLAS */
    err = clblasSetup( );
}


cl_context
getCLContext() {
    return ctx;
}

cl_command_queue
getCLQueue() {
    return queue;
}

#endif
